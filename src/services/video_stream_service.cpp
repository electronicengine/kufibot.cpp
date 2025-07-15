#include "video_stream_service.h"
#include "../logger.h"

VideoStreamService* VideoStreamService::_instance = nullptr;


VideoStreamService *VideoStreamService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new VideoStreamService();
    }
    return _instance;
}

cv::Mat VideoStreamService::take_snap_shot()
{
    if (!_cap.isOpened()) {
         _cap.open(_cameraIndex,cv::CAP_V4L2);
    }

    if (_subscribers.empty()) {
        _cap >> _frame;
    }
    
    return _frame;
}

VideoStreamService::VideoStreamService(int cameraIndex) : Service("VideoStreamService") , _cap(cameraIndex){
    if (!_cap.isOpened()) {
        Logger::error("Error: Could not open the camera.");
    }
}

VideoStreamService::~VideoStreamService() {
    stop();
    if (_serviceThread.joinable()) {
        _serviceThread.join(); 
    }
}

void VideoStreamService::service_function(){
    cv::Mat frame;

    _cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    _cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    if(!_cap.isOpened()){
        _cap.open(_cameraIndex, cv::CAP_V4L2);
        if(!_cap.isOpened()) {
            Logger::error("Error: Could not open the camera.");
            return;
        }
    }

    Logger::info("VideoStreamService is starting...");

    while (_running ) {

        _cap >> frame;
        _frame = frame;

        if (frame.empty()) {
            Logger::warn("Warning: Received empty frame!");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        VideoFrameData *data = new VideoFrameData();
        data->frame = frame;
        publish(MessageType::VideoFrame, data);
    }
}
