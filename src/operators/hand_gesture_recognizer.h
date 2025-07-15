#ifndef HAND_GESTURE_RECOGNIZER_H
#define HAND_GESTURE_RECOGNIZER_H

#include <Python.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

class HandGestureRecognizer {
public:
    HandGestureRecognizer(const std::string& venvPath);
    ~HandGestureRecognizer();

    bool initialize();
    bool processFrame(const cv::Mat& frame, std::string& gesture,
                      std::vector<int>& landmarks, std::vector<int>& bbox);

private:
    PyObject* pModule;
    PyObject* pFuncInit;
    PyObject* pFuncProcess;

    std::vector<int> pyListToIntVector(PyObject* list);
};

#endif // HAND_GESTURE_RECOGNIZER_H
