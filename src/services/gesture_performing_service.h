#ifndef  GESTURE_SERVICE_H
#define  GESTURE_SERVICE_H


#include <thread>
#include <atomic>

#include <string>
#include "service.h"
#include "robot_controller_service.h"
#include "../operators/speech_performing_operator.h"
#include "interactive_chat_service.h"
#include "web_socket_service.h"


class GesturePerformingService : public Service {

public:

    virtual ~GesturePerformingService();
    static GesturePerformingService *get_instance();


private:
    static GesturePerformingService *_instance;

    std::thread _workerThread;
    std::atomic<bool> _gestureWorking{false};
    GesturePerformingService();

    void greeter();
    void knowledgeable();
    void optimistic();
    void pessimistic();
    void curious();
    void service_function();

    //subscribed Data Functions
    void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data);
    void recognized_gesture(const std::string& _faceGesture, std::vector<int> _faceLandMark, const std::string& _handGesture, std::vector<int> _handLandmark);
    void llm_response(const std::string& response);

};

#endif