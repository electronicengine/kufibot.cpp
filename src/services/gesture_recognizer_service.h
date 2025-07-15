
#ifndef GESTURE_RECOGNIZER_SERVICE_H
#define GESTURE_RECOGNIZER_SERVICE_H

#include "service.h"
#include "../operators/face_gesture_recognizing_operator.h"
#include "../operators/hand_gesture_recognizing_operator.h"
#include <opencv2/opencv.hpp>
#include <map>
#include <string>
#include "../subscriber.h"
#include "../publisher.h"

class GestureRecognizerService : public Service, public Subscriber, public Publisher{
private:
    // Kamera ve işleme nesneleri
    cv::VideoCapture cap;
    FaceGestureRecognizingOperator *_faceGestureRecognizingOperator;
    HandGestureRecognizingOperator *_handGestureRecognizingOperator;

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

public:
    GestureRecognizerService(const std::string& name);
    ~GestureRecognizerService();

    bool initialize();
    void service_update_function() override;
    void update_video_frame(const cv::Mat& frame);
    void start();
    void stop();

};

#endif // GESTURE_RECOGNIZER_SERVICE_H