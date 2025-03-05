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
     std::map<std::string, int> jointAngles = {{"right_arm", 20}, {"left_arm", 170}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 170},{"eye_left", 20}};
    _robotControllerService->set_all_joint_angles(jointAngles);

    for(int i=0; i<2; i++){
        jointAngles = {{"right_arm", 20}, {"left_arm", 120}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 170},{"eye_left", 20}};
        _robotControllerService->set_all_joint_angles(jointAngles);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        jointAngles = {{"right_arm", 20}, {"left_arm", 150}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 170},{"eye_left", 20}};
        _robotControllerService->set_all_joint_angles(jointAngles);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

     }

}

void GestureService::knowledgeable()
{
    MainWindow::log("knowledgeable !", LogLevel::LOG_INFO);
     std::map<std::string, int> jointAngles = {{"right_arm", 20}, {"left_arm", 170}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 130},{"eye_left", 0}};
    _robotControllerService->set_all_joint_angles(jointAngles);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void GestureService::optimistic()
{
    MainWindow::log("optimistic !", LogLevel::LOG_INFO);
     std::map<std::string, int> jointAngles = {{"right_arm", 40}, {"left_arm", 140}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 170},{"eye_left", 20}};
    _robotControllerService->set_all_joint_angles(jointAngles);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void GestureService::pessimistic()
{
    MainWindow::log("pessimistic !", LogLevel::LOG_INFO);
     std::map<std::string, int> jointAngles = {{"right_arm", 20}, {"left_arm", 170}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 130},{"eye_left", 50}};
    _robotControllerService->set_all_joint_angles(jointAngles);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void GestureService::curious()
{
    MainWindow::log("curious !", LogLevel::LOG_INFO);
     std::map<std::string, int> jointAngles = {{"right_arm", 20}, {"left_arm", 170}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 170},{"eye_left", 20}};
    _robotControllerService->set_all_joint_angles(jointAngles);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    jointAngles = {{"right_arm", 20}, {"left_arm", 150}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 180},{"eye_left", 0}};
    _robotControllerService->set_all_joint_angles(jointAngles);

}

void GestureService::service_update_function()
{
    while(_running){

        std::string gesture;

        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            _cv.wait(lock, [this] { return !_gestureQueue.empty() || !_running; });

            if (!_running) break; // Exit if the worker is stopped

            gesture = _gestureQueue.front();
            _gestureQueue.pop();
        }

        if (gesture.find("selamlayan") != std::string::npos) {
            greeter();
        } else if (gesture.find("bilgili") != std::string::npos) {
            knowledgeable();
        } else if (gesture.find("iyimser") != std::string::npos) {
            optimistic();
        } else if (gesture.find("kötümser") != std::string::npos) {
            pessimistic();
        } else if (gesture.find("meraklı") != std::string::npos) {
            curious();
        } else {
            MainWindow::log("Unknown gesture: " + gesture, LogLevel::LOG_WARNING);
        }

    }
}

void GestureService::start()
{
    if (!_running) { // Ensure the thread is not already running
        _running = true;
        
        MainWindow::log("GestureService::_interactiveChatService::subscribe", LogLevel::LOG_TRACE);
        _interactiveChatService->subscribe(this);
        MainWindow::log("GestureService is starting..." , LogLevel::LOG_INFO);
        _serviceThread = std::thread(&GestureService::service_update_function, this);
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

    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        _gestureQueue.push(gesture); 
    }
    _cv.notify_one(); 

}