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
    cv::Mat frame;

    if (!_cap.isOpened()) {
         _cap.open(_cameraIndex,cv::CAP_V4L2);
    }

    if (_subscribers.empty()) {
        _cap >> frame;
    }
    
    return frame;
}

VideoStreamService::VideoStreamService(int cameraIndex) : Service("VideoStreamService") , _cap(cameraIndex){
}

VideoStreamService::~VideoStreamService() {
    stop();
    if (_serviceThread.joinable()) {
        _serviceThread.join(); 
    }
}

void VideoStreamService::service_function(){
    cv::Mat frame;

    _cap.open(_cameraIndex, cv::CAP_V4L2);
    if(!_cap.isOpened()) {
        ERROR("Error: Could not open the camera.");
        return;
    }

    _cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    _cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    INFO("VideoStreamService is starting...");

    while (_running ) {

        _cap >> frame;

        if (frame.empty()) {
            WARNING("Warning: Received empty frame!");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        std::unique_ptr<MessageData> data = std::make_unique<VideoFrameData>();
        static_cast<VideoFrameData*>(data.get())->frame = frame;
        publish(MessageType::VideoFrame, data);
    }
}
