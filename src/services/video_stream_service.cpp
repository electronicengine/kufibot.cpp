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

void VideoStreamService::start() {
    if (!_running) {

        _cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        _cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

        if(!_cap.isOpened()){
            _cap.open(_cameraIndex, cv::CAP_V4L2);
            if(!_cap.isOpened()) {
                Logger::error("Error: Could not open the camera.");
                return;
            }
        }

        _running = true;
        Logger::info("VideoStreamService is starting...");

        _serviceThread = std::thread(&VideoStreamService::service_update_function, this);
    }
}

void VideoStreamService::service_update_function(){
    cv::Mat frame;

    while (_running ) {

        // Wait until there are subscribers
        if (_subscribers.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }
        // Wait until cap is opened
        if(!_cap.isOpened()) {
            _cap.open(_cameraIndex, cv::CAP_V4L2);
            if (!_cap.isOpened())
                continue;
        }
        _cap >> frame;
        _frame = frame;

        if (frame.empty()) {
            Logger::warn("Warning: Received empty frame!");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        update_video_frame(frame);
    }
}

void VideoStreamService::stop() {

    if (_running){

        _running = false;

        Logger::info("VideoStreamService is stopping...");
        if (_serviceThread.joinable()) {
            _serviceThread.join();
        }
        _cap.release();
        Logger::info("VideoStreamService is stopped");
    }
}