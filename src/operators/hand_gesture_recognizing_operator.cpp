/*
* This file is part of Kufibot.
 *
 * Kufibot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kufibot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kufibot. If not, see <https://www.gnu.org/licenses/>.
 */

#include "hand_gesture_recognizing_operator.h"
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>



HandGestureRecognizingOperator::HandGestureRecognizingOperator(const std::string& venvPath)
    : pModule(nullptr), pFuncInit(nullptr), pFuncProcess(nullptr) {
    Py_Initialize();

    std::string sitePackages = venvPath + "/lib/python3.11/site-packages";
    std::string cmdPath = "import sys; sys.path.insert(0, '" + sitePackages + "')";
    PyRun_SimpleString(cmdPath.c_str());
    PyRun_SimpleString("import os; sys.path.insert(0, os.getcwd())");

    // Redirect Python stdout and stderr to a dummy class
    int saved_stderr;
    int dev_null;

    // Call this before running Python

    saved_stderr = dup(STDERR_FILENO);
    dev_null = open("/dev/null", O_WRONLY);
    dup2(dev_null, STDERR_FILENO);  // Redirect stderr to /dev/null


}

HandGestureRecognizingOperator::~HandGestureRecognizingOperator() {
    Py_XDECREF(pFuncInit);
    Py_XDECREF(pFuncProcess);
    Py_XDECREF(pModule);
    Py_Finalize();
}

bool HandGestureRecognizingOperator::initialize() {
    PyObject* pName = PyUnicode_DecodeFSDefault("hand_gesture_recognition_module");
    if (!pName) {
        PyErr_Print();
        return false;
    }

    pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if (!pModule) {
        PyErr_Print();
        return false;
    }

    pFuncInit = PyObject_GetAttrString(pModule, "initialize_detector");
    if (!pFuncInit || !PyCallable_Check(pFuncInit)) {
        PyErr_Print();
        return false;
    }

    PyObject* result = PyObject_CallObject(pFuncInit, NULL);
    if (!result) {
        PyErr_Print();
        return false;
    }
    Py_DECREF(result);

    pFuncProcess = PyObject_GetAttrString(pModule, "process_frame");
    if (!pFuncProcess || !PyCallable_Check(pFuncProcess)) {
        PyErr_Print();
        return false;
    }

    return true;
}

bool HandGestureRecognizingOperator::processFrame(const cv::Mat& frame, std::string& gesture,
                                         std::vector<int>& landmarks, std::vector<int>& bbox) {
    PyGILState_STATE gstate = PyGILState_Ensure(); // 🔒 GIL al

    PyObject* pyFrame = PyBytes_FromStringAndSize(
        reinterpret_cast<const char*>(frame.data),
        frame.total() * frame.elemSize());

    PyObject* args = PyTuple_New(4);
    PyTuple_SetItem(args, 0, pyFrame);
    PyTuple_SetItem(args, 1, PyLong_FromLong(frame.rows));
    PyTuple_SetItem(args, 2, PyLong_FromLong(frame.cols));
    PyTuple_SetItem(args, 3, PyLong_FromLong(frame.channels()));

    PyObject* result = PyObject_CallObject(pFuncProcess, args);
    Py_DECREF(args);

    if (!result) {
        PyErr_Print();
        PyGILState_Release(gstate); // 🔓 GIL bırak

        return false;
    }

    if (PyTuple_Check(result) && PyTuple_Size(result) == 3) {
        PyObject* pyGesture = PyTuple_GetItem(result, 0);
        PyObject* pyLms = PyTuple_GetItem(result, 1);
        PyObject* pyBBox = PyTuple_GetItem(result, 2);

        gesture = PyUnicode_AsUTF8(pyGesture);
        landmarks = pyListToIntVector(pyLms);
        bbox = pyListToIntVector(pyBBox);

        Py_DECREF(result);
        PyGILState_Release(gstate); // 🔓 GIL bırak

        return true;
    } else {
        Py_DECREF(result);
        PyGILState_Release(gstate); // 🔓 GIL bırak

        std::cerr << "Unexpected return from Python" << std::endl;
        return false;
    }
}

std::vector<int> HandGestureRecognizingOperator::pyListToIntVector(PyObject* list) {
    std::vector<int> result;
    if (PyList_Check(list)) {
        for (Py_ssize_t i = 0; i < PyList_Size(list); ++i) {
            PyObject* item = PyList_GetItem(list, i);
            if (PyLong_Check(item)) {
                result.push_back(PyLong_AsLong(item));
            }
        }
    }
    return result;
}
