#include "video_stream_service.h"
#include <iostream>
#include <algorithm> 


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

VideoStreamService::VideoStreamService(int cameraIndex) : _cap(cameraIndex){
    if (!_cap.isOpened()) {
        throw std::runtime_error("Error: Could not open the camera.");
    }
}

VideoStreamService::~VideoStreamService() {
    stop();
    if (_streamThread.joinable()) {
        _streamThread.join(); 
    }
}


void VideoStreamService::start() {
    if (!_running) { // Ensure the thread is not already running
        _running = true;
        std::cout << "VideoStreamService is starting..." << std::endl;
        _streamThread = std::thread(&VideoStreamService::streamLoop, this);   
    }
}

void VideoStreamService::streamLoop() {
    cv::Mat frame;

    if (!_cap.isOpened()) {
         _cap.open(0);
    }

    while (_running ) {
        if(!_subscribers.empty()){
            _cap >> frame;
            _frame = frame;
            if (frame.empty()) {
                std::cerr << "Warning: Received empty frame!" << std::endl;
                break;
            }
            update_video_frame(frame);  
        }
    }
}

void VideoStreamService::stop() {

    if (_running){

        _running = false;
        std::cout << "VideoStreamService is stopping..." << std::endl;
        if (_streamThread.joinable()) {
            _streamThread.join();  
        }
        _cap.release();  
        std::cout << "VideoStreamService is stopped" << std::endl;
    }
}