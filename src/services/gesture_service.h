#ifndef  GESTURE_SERVICE_H
#define  GESTURE_SERVICE_H


#include <thread>
#include <atomic>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <nlohmann/json.hpp>
#include <iostream>
#include <functional>
#include <map> 
#include <unordered_map>
#include <chrono>
#include <thread>
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <regex>
#include <atomic>

#include "../subscriber.h"
#include "../publisher.h"
#include "service.h"
#include "robot_controller_service.h"
#include "../controllers/speech_process_controller.h"
#include "../controllers/speech_recognition_controller.h"
#include "../controllers/curl_controller.h"
#include "../controllers/execution_controller.h"
#include "../controllers/dc_motor_controller.h"
#include "mapping_service.h"

#include "video_stream_service.h"
#include "interactive_chat_service.h"
#include "web_socket_service.h"


class GestureService : public Subscriber, public Service {

private:
    static GestureService *_instance;
    RobotControllerService *_robotControllerService;
    InteractiveChatService *_interactiveChatService;
    std::queue<std::string> _gestureQueue; 
    std::mutex _queueMutex;                
    std::condition_variable _cv;           
    std::thread _workerThread;             
    std::atomic<bool> _gestureWorking{false};
    GestureService();

public:

    static GestureService *get_instance();
    ~GestureService();

    void greeter();
    void knowledgeable();
    void optimistic();
    void pessimistic();
    void curious();
    void service_update_function();

    void update_gesture(const std::string& gesture);
    void start();
    void stop(); 
};

#endif