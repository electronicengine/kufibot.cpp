// VideoStream.h
#ifndef VIDEO_STREAM_SERVICE_H
#define VIDEO_STREAM_SERVICE_H

#include <opencv2/opencv.hpp>
#include "service.h"

class VideoStreamService : public Service{

public:
    virtual ~VideoStreamService();

    static VideoStreamService *get_instance();


private:
    cv::VideoCapture _cap;
    int _cameraIndex;
    static VideoStreamService* _instance;
    cv::Mat _frame;

    VideoStreamService(int cameraIndex = 0);
    void streamLoop();
    cv::Mat take_snap_shot();
    void service_function();

    //subscribed to noting
    void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data){(void) type, (void) data; };

};

#endif // VIDEOSTREAM_H