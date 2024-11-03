#ifndef INTERACTIVE_CHAT_SERVICE_H
#define INTERACTIVE_CHAT_SERVICE_H


#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
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

#include "../subscriber.h"
#include "service.h"
#include "robot_controller_service.h"
#include "../controllers/speech_process_controller.h"
#include "../controllers/speech_recognition_controller.h"
#include "../controllers/curl_controller.h"
#include "../controllers/execution_controller.h"
#include "../controllers/dc_motor_controller.h"
#include "mapping_service.h"

#include "video_stream_service.h"

#include "web_socket_service.h"

using Json = nlohmann::json;


class InteractiveChatService : public Subscriber, public Service {

private:
    static InteractiveChatService *_instance;
    RobotControllerService *_robotControllerService;
    SpeechRecognitionController *_speechRecognitionController;
    SpeechProcessController *_speechProcessController;
    WebSocketService *_webSocketService;
    CurlController *_curlController;
    ExecutionController *_executionController;
    VideoStreamService *_videoStreamService;
    
    InteractiveChatService();

    const std::string translate(const std::string& source, const std::string& target, const std::string& Text);
    std::vector<std::string> splitSentences(std::string text);
    void walkie_talkie_thread(const std::string& message);

public:

    static InteractiveChatService *get_instance();
    ~InteractiveChatService();
    void service_update_function();
    void update_web_socket_message(websocketpp::connection_hdl hdl,  const std::string& mg);
    void start();
    void stop(); 
    

};

#endif