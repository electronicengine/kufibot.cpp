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
#include "robot_controller_service.h"
#include "../controllers/speech_process_controller.h"
#include "../controllers/speech_recognition_controller.h"
#include "../controllers/curl_controller.h"
#include "web_socket_service.h"

using Json = nlohmann::json;


class InteractiveChatService : public Subscriber {

private:
    std::atomic<bool> _running{false}; 
    static InteractiveChatService *_instance;
    RobotControllerService *_robotControllerService;
    SpeechRecognitionController *_speechRecognitionController;
    SpeechProcessController *_speechProcessController;
    WebSocketService *_webSocketService;
    CurlController *_curlController;
    std::thread _chat_thread; 
    std::mutex _walkie_thread_mutex;

    InteractiveChatService();
    void chat_loop();

    std::string executeCommand(const std::string& command);
    const std::string translate(const std::string& source, const std::string& target, const std::string& Text);
    std::vector<std::string> splitSentences(std::string text);
    void walkie_talkie_thread(const std::string& message);

public:

    static InteractiveChatService *get_instance();
    ~InteractiveChatService();

    void update_web_socket_message(websocketpp::connection_hdl hdl,  const std::string& mg);
    void start();
    void stop(); 
    

};

#endif