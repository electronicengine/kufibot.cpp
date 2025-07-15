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
#include "../operators/speech_performing_operator.h"
#include "../operators/speech_recognizing_operator.h"
#include "../operators/http_request_operator.h"
#include "../operators/cmdline_execution_operator.h"
#include "../controllers/dc_motor_controller.h"
#include "../operators/llama_operator.h"
#include "mapping_service.h"

#include "video_stream_service.h"

#include "web_socket_service.h"

struct LlamaOptions {
    std::string llamaChatModelPath;
    std::string llamaEmbeddingModelPath;
    double temperature;
    int maxTokenSize;
    double topK;
    double topP;
    int nThreads;
    int poolingType;
};



class InteractiveChatService : public Publisher, public Subscriber, public Service {

private:
    static InteractiveChatService *_instance;
    RobotControllerService *_robotControllerService;
    SpeechRecognizingOperator *_speechRecognizingOperator;
    SpeechPerformingOperator * _speechPerformingOperator;
    WebSocketService *_webSocketService;
    VideoStreamService *_videoStreamService;
    std::string _ollamaServer;
    std::string _ollamaModelName;
    std::atomic<bool> _queryRunning{false};
    std::string _responseStr;
    std::deque<std::string> _stringQueue;
    std::mutex _queueMutex;
    std::condition_variable _queueCondition;
    LlamaOptions _llamaOptions;
    LlamaOperator _llamaChatOperator;

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
    bool load_model(const LlamaOptions &llamaOptions);
    void service_update_function();
};

#endif