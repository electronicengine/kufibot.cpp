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


#include "robot_controller_service.h"
#include "../controllers/speech_process_controller.h"
#include "../controllers/speech_recognition_controller.h"
#include "../controllers/curl_controller.h"

using Json = nlohmann::json;


class InteractiveChatService{

private:
    std::atomic<bool> _running{false}; 
    static InteractiveChatService *_instance;
    RobotControllerService *_robotControllerService;
    SpeechRecognitionController *_speechRecognitionController;
    SpeechProcessController *_speechProcessController;
    CurlController *_curlController;
    std::thread _chat_thread; 
    
    InteractiveChatService();
    void chat_loop();

    std::string executeCommand(const std::string& command);
    const std::string translate(const std::string& source, const std::string& target, const std::string& Text);
    std::vector<std::string> splitSentences(std::string text);
public:

    static InteractiveChatService *get_instance();
    ~InteractiveChatService();

    void start();
    void stop(); 
    

};

#endif