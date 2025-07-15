#include "gesture_performing_service.h"
#include "../logger.h"

GesturePerformingService* GesturePerformingService::_instance = nullptr;

GesturePerformingService *GesturePerformingService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new GesturePerformingService();
    }
    return _instance;
}


GesturePerformingService::GesturePerformingService() : Service("GesturePerformingService") {

    _robotControllerService = RobotControllerService::get_instance();
    _interactiveChatService = InteractiveChatService::get_instance();
    _gestureWorking = false;

}


GesturePerformingService::~GesturePerformingService()
{

}


void GesturePerformingService::greeter()
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

void GesturePerformingService::knowledgeable()
{
    Logger::info("knowledgeable !");
    std::map<std::string, int> jointAngles = {{"right_arm", 20}, {"left_arm", 170}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 130},{"eye_left", 0}};
    _robotControllerService->set_all_joint_angles(jointAngles);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void GesturePerformingService::optimistic()
{
    Logger::info("optimistic !");
     std::map<std::string, int> jointAngles = {{"right_arm", 40}, {"left_arm", 140}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 170},{"eye_left", 20}};
    _robotControllerService->set_all_joint_angles(jointAngles);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void GesturePerformingService::pessimistic()
{
    Logger::info("pessimistic !");
    std::map<std::string, int> jointAngles = {{"right_arm", 20}, {"left_arm", 170}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 130},{"eye_left", 50}};
    _robotControllerService->set_all_joint_angles(jointAngles);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void GesturePerformingService::curious()
{
    Logger::info("curious !");
    std::map<std::string, int> jointAngles = {{"right_arm", 20}, {"left_arm", 170}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 170},{"eye_left", 20}};
    _robotControllerService->set_all_joint_angles(jointAngles);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    jointAngles = {{"right_arm", 20}, {"left_arm", 150}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 180},{"eye_left", 0}};
    _robotControllerService->set_all_joint_angles(jointAngles);

}

void GesturePerformingService::service_update_function()
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

void GesturePerformingService::start()
{
    if (!_running) { // Ensure the thread is not already running
        _running = true;
        Logger::info("GesturePerformingService::_interactiveChatService::subscribe");
        _interactiveChatService->subscribe(this);
        Logger::info("GesturePerformingService is starting...");
        _serviceThread = std::thread(&GesturePerformingService::service_update_function, this);
    }
}

void GesturePerformingService::stop()
{
    if (_running){
        _running = false;
        Logger::info("GesturePerformingService is stopping...");
        _interactiveChatService->un_subscribe(this);
        Logger::info("GesturePerformingService::_interactiveChatService::un_subscribe");

        if (_serviceThread.joinable()) {
            _serviceThread.join(); 
        }

        Logger::info("GesturePerformingService is stopped.");
    }
}


void GesturePerformingService::update_gesture(const std::string &gesture)
{
    Logger::info("update_gesture : {}", gesture);

    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        _gestureQueue.push(gesture); 
    }
    _cv.notify_one(); 

}