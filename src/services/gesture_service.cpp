#include "gesture_service.h"
#include "../ui/main_window.h"

GestureService* GestureService::_instance = nullptr;

GestureService *GestureService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new GestureService();
    }
    return _instance;
}


GestureService::GestureService() : Service("GestureService") {

    _robotControllerService = RobotControllerService::get_instance();
    _interactiveChatService = InteractiveChatService::get_instance();
    _gestureWorking = false;

}


GestureService::~GestureService()
{

}

void GestureService::greeter()
{
    MainWindow::log("greeter !", LogLevel::LOG_INFO);
    _robotControllerService->control_arm("right_arm", 20, false);

    for(int i=0; i<3; i++){
        _robotControllerService->control_arm("left_arm", 120, false);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        _robotControllerService->control_arm("left_arm", 150, false);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

     }

}

void GestureService::knowledgeable()
{

}

void GestureService::optimistic()
{

}

void GestureService::pessimistic()
{

}

void GestureService::curious()
{

}


void GestureService::start()
{
    if (!_running) { // Ensure the thread is not already running
        _running = true;
        
        MainWindow::log("GestureService::_interactiveChatService::subscribe", LogLevel::LOG_TRACE);
        _interactiveChatService->subscribe(this);
        MainWindow::log("GestureService is starting..." , LogLevel::LOG_INFO);

    }
}

void GestureService::stop()
{
    if (_running){
        _running = false;
        MainWindow::log("GestureService is stopping..." , LogLevel::LOG_INFO);
        _interactiveChatService->un_subscribe(this);
        MainWindow::log("GestureService::_interactiveChatService::un_subscribe", LogLevel::LOG_TRACE);

        if (_serviceThread.joinable()) {
            _serviceThread.join(); 
        }

        MainWindow::log("GestureService is stopped." , LogLevel::LOG_INFO);
    }
}

void GestureService::update_gesture(const std::string &gesture)
{
    MainWindow::log("update_gesture : " + gesture, LogLevel::LOG_INFO);

    // If already working on a gesture, do nothing
    if (_gestureWorking.exchange(true)) { // Atomically check and set to true
        return;
    }

    if (gesture.find("selamlayan") != std::string::npos) {
        std::thread gestureThread([this]() {
            greeter();
            _gestureWorking = false; // Reset the flag when the gesture is done
        });
        gestureThread.detach();
    } else {
        _gestureWorking = false; // Reset if the gesture doesn't match
    }
}