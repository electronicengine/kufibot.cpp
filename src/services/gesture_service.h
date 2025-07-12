#ifndef  GESTURE_SERVICE_H
#define  GESTURE_SERVICE_H


#include <thread>
#include <atomic>

#include <string>
#include "../subscriber.h"
#include "service.h"
#include "robot_controller_service.h"
#include "../controllers/speech_process_controller.h"
#include "mapping_service.h"
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