// VideoStream.h
#ifndef VIDEO_STREAM_SERVICE_H
#define VIDEO_STREAM_SERVICE_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>

#include "service.h"
#include "../publisher.h"

class VideoStreamService : public Publisher, public Service{
private:
    cv::VideoCapture _cap;
    int _cameraIndex;
    static VideoStreamService* _instance;
    cv::Mat _frame;
    VideoStreamService(int cameraIndex = 0);
    void streamLoop();  

public:
    ~VideoStreamService();  

    static VideoStreamService *get_instance();
    cv::Mat take_snap_shot();
    void service_update_function();
    void start();  
    void stop();  
};

#endif // VIDEOSTREAM_H