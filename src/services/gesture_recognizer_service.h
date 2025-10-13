
#ifndef GESTURE_RECOGNIZER_SERVICE_H
#define GESTURE_RECOGNIZER_SERVICE_H

#include "service.h"
#include "../operators/face_gesture_recognizing_operator.h"
#include "../operators/hand_gesture_recognizing_operator.h"
#include <opencv2/opencv.hpp>
#include <map>
#include <string>


class GestureRecognizerService : public Service{

public:
    virtual ~GestureRecognizerService();
    static GestureRecognizerService *get_instance();


private:
    // Kamera ve işleme nesneleri
    cv::VideoCapture cap;
    FaceGestureRecognizingOperator *_faceGestureRecognizingOperator;
    HandGestureRecognizingOperator *_handGestureRecognizingOperator;

    // Görüntüleme ayarları
    bool showFaceMesh;
    bool showFaceInfo;
    bool showHandLandmarks;
    bool _initialized;

    // Python ortam yolu
    std::string venvPath;

    static GestureRecognizerService *_instance;

    GestureRecognizerService(const std::string& name = "GestureRecognizerService");


    // İşleme fonksiyonları
    void processFrame(cv::Mat& frame);
    std::map<std::string, float> parseFaceInfoString(const std::string& faceInfoStr);
    void displayFPS(cv::Mat& frame, double& ptime);

    bool initialize();
    void service_function();
    std::queue<cv::Mat> _frameQueue;

    //subscribed video_frame
    void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data);

    void video_frame(cv::Mat& frame);

};

#endif // GESTURE_RECOGNIZER_SERVICE_H