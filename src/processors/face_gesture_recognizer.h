#ifndef FACE_GESTURE_RECOGNIZER_H
#define FACE_GESTURE_RECOGNIZER_H

#include <Python.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

class FaceGestureRecognizer {
public:
    FaceGestureRecognizer(const std::string& venvPath);
    ~FaceGestureRecognizer();
    
    bool initialize();
    bool processFrame(const cv::Mat& frame, std::string& emotion, 
                     std::vector<int>& landmarks, std::string& faceInfo);

private:
    PyObject* pModule;
    PyObject* pFuncInit;
    PyObject* pFuncProcess;
    
    std::vector<int> pyListToIntVector(PyObject* list);
};

#endif // FACE_GESTURE_RECOGNIZER_H