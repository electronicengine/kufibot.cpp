#include "face_gesture_recognizer.h"
#include <iostream>

FaceGestureRecognizer::FaceGestureRecognizer(const std::string& venvPath)
    : pModule(nullptr), pFuncInit(nullptr), pFuncProcess(nullptr) {
    Py_Initialize();
    
    // Virtual environment path'i Python'a ekle
    std::string sitePackages = venvPath + "/lib/python3.11/site-packages";
    std::string cmdPath = "import sys; sys.path.insert(0, '" + sitePackages + "')";
    PyRun_SimpleString(cmdPath.c_str());
    
    // Mevcut dizini de path'e ekle
    PyRun_SimpleString("import os; sys.path.insert(0, os.getcwd())");
}

FaceGestureRecognizer::~FaceGestureRecognizer() {
    Py_XDECREF(pFuncInit);
    Py_XDECREF(pFuncProcess);
    Py_XDECREF(pModule);
    Py_Finalize();
}

bool FaceGestureRecognizer::initialize() {
    // Python modülünü import et
    PyObject* pName = PyUnicode_DecodeFSDefault("face_gesture_recognition_module");
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
    
    // Initialize fonksiyonunu al
    pFuncInit = PyObject_GetAttrString(pModule, "initialize_detector_face");
    if (!pFuncInit || !PyCallable_Check(pFuncInit)) {
        PyErr_Print();
        return false;
    }
    
    // Initialize fonksiyonunu çağır
    PyObject* result = PyObject_CallObject(pFuncInit, NULL);
    if (!result) {
        PyErr_Print();
        return false;
    }
    Py_DECREF(result);
    
    // Process fonksiyonunu al
    pFuncProcess = PyObject_GetAttrString(pModule, "process_frame_face");
    if (!pFuncProcess || !PyCallable_Check(pFuncProcess)) {
        PyErr_Print();
        return false;
    }
    
    return true;
}

bool FaceGestureRecognizer::processFrame(const cv::Mat& frame, std::string& emotion,
                                        std::vector<int>& landmarks, std::string& faceInfo) {
    // OpenCV Mat'i Python'a göndermek için byte array'e dönüştür
    PyObject* pyFrame = PyBytes_FromStringAndSize(
        reinterpret_cast<const char*>(frame.data),
        frame.total() * frame.elemSize());
    
    // Python fonksiyonuna gönderilecek argümanları hazırla
    PyObject* args = PyTuple_New(4);
    PyTuple_SetItem(args, 0, pyFrame);
    PyTuple_SetItem(args, 1, PyLong_FromLong(frame.rows));
    PyTuple_SetItem(args, 2, PyLong_FromLong(frame.cols));
    PyTuple_SetItem(args, 3, PyLong_FromLong(frame.channels()));
    
    // Python fonksiyonunu çağır
    PyObject* result = PyObject_CallObject(pFuncProcess, args);
    Py_DECREF(args);
    
    if (!result) {
        PyErr_Print();
        return false;
    }
    
    // Sonucu ayrıştır
    if (PyTuple_Check(result) && PyTuple_Size(result) == 3) {
        PyObject* pyEmotion = PyTuple_GetItem(result, 0);
        PyObject* pyLandmarks = PyTuple_GetItem(result, 1);
        PyObject* pyFaceInfo = PyTuple_GetItem(result, 2);
        
        // Emotion string'i al
        if (PyUnicode_Check(pyEmotion)) {
            emotion = PyUnicode_AsUTF8(pyEmotion);
        }
        
        // Landmarks'ları al
        landmarks = pyListToIntVector(pyLandmarks);
        
        // Face info string'i al
        if (PyUnicode_Check(pyFaceInfo)) {
            faceInfo = PyUnicode_AsUTF8(pyFaceInfo);
        }
        
        Py_DECREF(result);
        return true;
    } else {
        Py_DECREF(result);
        std::cerr << "Unexpected return from Python function" << std::endl;
        return false;
    }
}

std::vector<int> FaceGestureRecognizer::pyListToIntVector(PyObject* list) {
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