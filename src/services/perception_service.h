
#ifndef PERCEPTION_SERVICE_H
#define PERCEPTION_SERVICE_H

#include "service.h"
#include "../operators/face_gesture_recognizing_operator.h"
#include "../operators/hand_gesture_recognizing_operator.h"
#include "../operators/video_operator.h"
#include <opencv2/opencv.hpp>
#include <atomic>
#include <map>
#include <memory>
#include <string>


class PerceptionService : public Service{

public:
    ~PerceptionService() override;
    static PerceptionService *get_instance(bool showFrame = false, const std::string& name = "PerceptionService");


private:
    FaceGestureRecognizingOperator _faceGestureRecognizingOperator;
    HandGestureRecognizingOperator _handGestureRecognizingOperator;
    VideoOperator _videoOperator;
    bool _showFrame;
    std::atomic<bool> _aiModeEnabled{true};

    static PerceptionService *_instance;

    explicit PerceptionService(const std::string& name = "PerceptionService",  bool showFrame = false);

    void processFrame(cv::Mat& frame);
    void processSpeechInput();
    std::map<std::string, float> parseFaceInfoString(const std::string& faceInfoStr);
    void displayFPS(cv::Mat& frame, double& ptime);
    void publishVideoFrame(const cv::Mat& frame);

    bool initialize() override;
    void service_function() override;

    void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data) override;

public:
    VideoOperator& get_video_operator();
    std::shared_ptr<cv::VideoCapture> get_capture();

};

#endif // PERCEPTION_SERVICE_H
