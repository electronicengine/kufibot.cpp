#include "video_stream_service.h"
#include <iostream>
#include <algorithm> 
#include "../ui/main_window.h"

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
         _cap.open(0);
    }

    if (_subscribers.empty()) {
        _cap >> _frame;
    }
    
    return _frame;
}

VideoStreamService::VideoStreamService(int cameraIndex) : Service("VideoStreamService") , _cap(cameraIndex){
    if (!_cap.isOpened()) {
        throw std::runtime_error("Error: Could not open the camera.");
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
        _running = true;
        MainWindow::log("VideoStreamService is starting...", LogLevel::LOG_INFO);
        _serviceThread = std::thread(&VideoStreamService::service_update_function, this);   
    }
}

void VideoStreamService::service_update_function(){
    cv::Mat frame;

    if (!_cap.isOpened()) {
         _cap.open(0);
    }

    while (_running ) {
        if(!_subscribers.empty()){
            _cap >> frame;
            _frame = frame;
            if (frame.empty()) {
                MainWindow::log("Warning: Received empty frame!", LogLevel::LOG_WARNING);
                break;
            }
            update_video_frame(frame);  
        }
    }
}

void VideoStreamService::stop() {

    if (_running){

        _running = false;
        MainWindow::log("VideoStreamService is stopping...", LogLevel::LOG_INFO);
        if (_serviceThread.joinable()) {
            _serviceThread.join();  
        }
        _cap.release();  
        MainWindow::log("VideoStreamService is stopped", LogLevel::LOG_INFO);
    }
}