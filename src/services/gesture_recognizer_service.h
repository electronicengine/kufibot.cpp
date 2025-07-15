
#ifndef GESTURE_RECOGNIZER_SERVICE_H
#define GESTURE_RECOGNIZER_SERVICE_H

#include "service.h"
#include "../operators/face_gesture_recognizer.h"
#include "../operators/hand_gesture_recognizer.h"
#include <opencv2/opencv.hpp>
#include <map>
#include <string>
#include <vector>

class GestureRecognizerService : public Service {
private:
    // Kamera ve işleme nesneleri
    cv::VideoCapture cap;
    FaceGestureRecognizer faceRecognizer;
    HandGestureRecognizer handRecognizer;

    // Görüntüleme ayarları
    bool showFaceMesh;
    bool showFaceInfo;
    bool showHandLandmarks;

    // Python ortam yolu
    std::string venvPath;

    // İşleme fonksiyonları
    void processFrame(cv::Mat& frame);
    std::map<std::string, float> parseFaceInfoString(const std::string& faceInfoStr);
    void displayFPS(cv::Mat& frame, double& ptime);
    void handleKeyPress(int key);

public:
    GestureRecognizerService(const std::string& name, const std::string& venvPath);
    ~GestureRecognizerService();

    bool initialize();
    void service_update_function() override;
};

#endif // GESTURE_RECOGNIZER_SERVICE_H