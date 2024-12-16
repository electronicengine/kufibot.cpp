#ifndef INTERACTIVE_CHAT_SERVICE_H
#define INTERACTIVE_CHAT_SERVICE_H

#include "ollama.hpp"
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

#include "web_socket_service.h"


class InteractiveChatService : public Publisher, public Subscriber, public Service {

private:
    static InteractiveChatService *_instance;
    RobotControllerService *_robotControllerService;
    SpeechRecognitionController *_speechRecognitionController;
    SpeechProcessController *_speechProcessController;
    WebSocketService *_webSocketService;
    CurlController *_curlController;
    ExecutionController *_executionController;
    VideoStreamService *_videoStreamService;
    std::string _llamaServer;
    std::string _modelName;
    std::atomic<bool> _queryRunning{false};
    std::string _responseStr;
    std::deque<std::string> _stringQueue;
    std::mutex _queueMutex;
    std::condition_variable _queueCondition;

    InteractiveChatService();
    const std::string translate(const std::string& source, const std::string& target, const std::string& Text);
    std::vector<std::string> splitSentences(std::string text);
    void push_speak_string(const std::string& text);

public:

    static InteractiveChatService *get_instance();
    ~InteractiveChatService();
    void update_web_socket_message(websocketpp::connection_hdl hdl,  const std::string& mg);
    void start();
    void stop(); 
    bool query(const std::string& message, std::function<void(const std::string&)> onReceiveLlamaResponse);
    void set_llama_server(const std::string& server);
    void load_model();
    void service_update_function();
};

#endif