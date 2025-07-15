#ifndef HAND_GESTURE_RECOGNIZING_OPERATOR_H
#define HAND_GESTURE_RECOGNIZING_OPERATOR_H

#include <Python.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

class HandGestureRecognizingOperator {
public:
    ~HandGestureRecognizingOperator();

    bool initialize();
    bool processFrame(const cv::Mat& frame, std::string& gesture,
                      std::vector<int>& landmarks, std::vector<int>& bbox);
    static HandGestureRecognizingOperator* get_instance(const std::string& venvPath = "/home/kufi/venv");

private:
    PyObject* pModule;
    PyObject* pFuncInit;
    PyObject* pFuncProcess;
    static HandGestureRecognizingOperator* _instance;
    HandGestureRecognizingOperator(const std::string& venvPath);
    std::vector<int> pyListToIntVector(PyObject* list);
};

#endif // HAND_GESTURE_RECOGNIZING_OPERATOR_H
