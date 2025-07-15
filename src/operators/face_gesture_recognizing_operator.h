#ifndef FACE_GESTURE_RECOGNIZING_OPERATOR_H
#define FACE_GESTURE_RECOGNIZING_OPERATOR_H

#include <Python.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

class FaceGestureRecognizingOperator {
public:
    ~FaceGestureRecognizingOperator();
    
    bool initialize();
    bool processFrame(const cv::Mat& frame, std::string& emotion, 
                     std::vector<int>& landmarks, std::string& faceInfo);
    static FaceGestureRecognizingOperator* get_instance(const std::string& venvPath = "/home/kufi/venv");

private:
    PyObject* pModule;
    PyObject* pFuncInit;
    PyObject* pFuncProcess;

    static FaceGestureRecognizingOperator* _instance;

    FaceGestureRecognizingOperator(const std::string& venvPath);
    std::vector<int> pyListToIntVector(PyObject* list);
};

#endif // FACE_GESTURE_RECOGNIZING_OPERATOR_H