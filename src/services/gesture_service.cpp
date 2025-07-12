#include "gesture_service.h"
#include "../logger.h"

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
    
    Logger::info("greeter !");
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
    Logger::info("knowledgeable !");
    std::map<std::string, int> jointAngles = {{"right_arm", 20}, {"left_arm", 170}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 130},{"eye_left", 0}};
    _robotControllerService->set_all_joint_angles(jointAngles);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void GestureService::optimistic()
{
    Logger::info("optimistic !");
     std::map<std::string, int> jointAngles = {{"right_arm", 40}, {"left_arm", 140}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 170},{"eye_left", 20}};
    _robotControllerService->set_all_joint_angles(jointAngles);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void GestureService::pessimistic()
{
    Logger::info("pessimistic !");
    std::map<std::string, int> jointAngles = {{"right_arm", 20}, {"left_arm", 170}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 130},{"eye_left", 50}};
    _robotControllerService->set_all_joint_angles(jointAngles);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void GestureService::curious()
{
    Logger::info("curious !");
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
            Logger::error("Unknown gesture:");
        }
    }
}

void GestureService::start()
{
    if (!_running) { // Ensure the thread is not already running
        _running = true;
        Logger::info("GestureService::_interactiveChatService::subscribe");
        _interactiveChatService->subscribe(this);
        Logger::info("GestureService is starting...");
        _serviceThread = std::thread(&GestureService::service_update_function, this);
    }
}

void GestureService::stop()
{
    if (_running){
        _running = false;
        Logger::info("GestureService is stopping...");
        _interactiveChatService->un_subscribe(this);
        Logger::info("GestureService::_interactiveChatService::un_subscribe");

        if (_serviceThread.joinable()) {
            _serviceThread.join(); 
        }

        Logger::info("GestureService is stopped.");
    }
}


void GestureService::update_gesture(const std::string &gesture)
{
    Logger::info("update_gesture : {}", gesture);

    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        _gestureQueue.push(gesture); 
    }
    _cv.notify_one(); 

}