#include "interactive_chat_service.h"
#include "../ui/main_window.h"

InteractiveChatService* InteractiveChatService::_instance = nullptr;

InteractiveChatService *InteractiveChatService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new InteractiveChatService();
    }
    return _instance;
}


InteractiveChatService::InteractiveChatService() : Service("InteractiveChatService") {
    _speechProcessController = SpeechProcessController::get_instance();
    _speechRecognitionController = SpeechRecognitionController::get_instance();
    _robotControllerService = RobotControllerService::get_instance();
    _webSocketService = WebSocketService::get_instance();
    _executionController = ExecutionController::get_instance();
    _videoStreamService = VideoStreamService::get_instance();

    _llamaServer = "http://192.168.1.20:11434";
    _modelName = "kufi";

    _executionController->set_venv("/home/kufi/venv");
    // _curlController = CurlController::get_instance("http://192.168.1.20:11434/api/generate");
    _curlController = CurlController::get_instance("https://generativelanguage.googleapis.com/v1beta/tunedModels/kufi-2165:generateContent?key=AIzaSyAj3z8oiHbljABcdsKRAgO05d7zcNS9Bsw");
    _speechProcessController->loadModel("../ai.models/trSpeechModel/dfki.onnx",
                    "../ai.models/trSpeechModel/dfki.onnx.json");
    _speechRecognitionController->load_model("../ai.models/trRecognizeModel");
    if(!_speechRecognitionController->open()) {
        return ;
    }
}


const std::string InteractiveChatService::translate(const std::string& source, const std::string& target, const std::string& text){

    std::string venvPath = "/home/kufi/venv/bin/activate";
    std::string scriptPath = "/home/kufi/workspace/translate_test.py";

    std::string command ="trans -b " + source + ":" + target + " \"" + text + "\"";
    std::string translatedText = _executionController->run(command);

    return translatedText;
}


InteractiveChatService::~InteractiveChatService()
{
}

bool InteractiveChatService::query(const std::string& message, std::function<void(const std::string&)> onReceiveLlamaResponse){
    std::lock_guard<std::mutex> lock(_dataMutex);

    if (_queryRunning) {
        return false;
    } else {

        std::function<void(const ollama::response&)> response_callback =
            [this, onReceiveLlamaResponse](const ollama::response& response) {

                std::string partial = response.as_simple_string();

                _responseStr += partial;

                if (partial.find("#") != std::string::npos) {
                    std::string sentence;
                    std::string gesture;
                    size_t start = _responseStr.find('/') + 1;
                    size_t end = _responseStr.find('#');    

                    if (start != std::string::npos && end != std::string::npos && start < end) {
                        sentence = _responseStr.substr(0, start -1);
                        gesture = _responseStr.substr(start, end - start); 
                    }

                    push_speak_string(sentence);
                    onReceiveLlamaResponse(sentence);
                    _responseStr.clear();
                }

                if (response.as_json()["done"]==true) { _queryRunning = false; _responseStr.clear();}
            };

        std::thread new_thread([this, response_callback, message]() {
            ollama::generate(_modelName, message, response_callback);
        });

        new_thread.detach();

        _queryRunning = true;
        return true;
    }
}

void InteractiveChatService::set_llama_server(const std::string &server)
{
    size_t dashPos = server.find('-');

    if (dashPos != std::string::npos) {
        _llamaServer = server.substr(0, dashPos);
        _modelName = server.substr(dashPos + 1);

        ollama::setServerURL(_llamaServer);    
        MainWindow::log( "set llama server: " + _llamaServer , LogLevel::LOG_TRACE);
        MainWindow::log( "set model name: " + _modelName , LogLevel::LOG_TRACE);


    } else {
        MainWindow::log( "the server address couldn't parsed" , LogLevel::LOG_ERROR);
    }

}

void InteractiveChatService::load_model()
{
    MainWindow::log("InteractiveChatService:: llama model loading...", LogLevel::LOG_TRACE);

    bool model_loaded = ollama::load_model(_modelName);
    if (model_loaded) 
        MainWindow::log("InteractiveChatService:: llama model loaded!", LogLevel::LOG_TRACE);
    else
        MainWindow::log("InteractiveChatService:: llama model couldn't loaded!", LogLevel::LOG_ERROR);
}

void InteractiveChatService::service_update_function()
{
    while (_running) {
        std::string textToSpeak;

        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            _queueCondition.wait(lock, [this] {
                return !_stringQueue.empty() || !_running;
            });
            if (!_running && _stringQueue.empty()) {
                break;
            }
            textToSpeak = _stringQueue.front();
            _stringQueue.pop_front();
        }
        if (!textToSpeak.empty()) {
            _speechProcessController->speakText(textToSpeak);
        }
    }
}


void InteractiveChatService::push_speak_string(const std::string& text) {
    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        _stringQueue.push_back(text);
    }
    _queueCondition.notify_one();
}

void InteractiveChatService::update_web_socket_message(websocketpp::connection_hdl hdl, const std::string &msg)
{
    if(msg == "on_open"){
        MainWindow::log( "new web socket connection extablished"  , LogLevel::LOG_TRACE);

    }
    else if(msg == "on_close"){
        MainWindow::log( "web socket connection is closed." , LogLevel::LOG_TRACE);
    }
    else{
        Json message = Json::parse(msg);
        if (message.contains("talkie")) {
            std::string incoming_talkie = message["talkie"];
            MainWindow::log( "incoming_talkie: " + incoming_talkie , LogLevel::LOG_TRACE);

        std::function<void(const ollama::response&)> response_callback =
            [=](const ollama::response& response) {
                MainWindow::log(response, LogLevel::LOG_TRACE);
                if (response.as_json()["done"]==true) { _queryRunning =true; }

            };

            query(incoming_talkie, response_callback);


        }
    }
}

void InteractiveChatService::start()
{
    if (!_running) { // Ensure the thread is not already running
        _running = true;
        ollama::setServerURL(_llamaServer);    

        ollama::setReadTimeout(120);
        ollama::setWriteTimeout(120);

        _serviceThread = std::thread(&InteractiveChatService::service_update_function, this);
        MainWindow::log("InteractiveChatService::_webSocketService::subscribe", LogLevel::LOG_TRACE);
        _webSocketService->subscribe(this);
        MainWindow::log("InteractiveChatService is starting..." , LogLevel::LOG_INFO);

    }
}

void InteractiveChatService::stop()
{
    if (_running){
        _running = false;
        MainWindow::log("InteractiveChatService is stopping..." , LogLevel::LOG_INFO);

        _speechRecognitionController->stop_listen();
        _speechRecognitionController->close();

        if (_serviceThread.joinable()) {
            _serviceThread.join(); 
        }
            {
        std::lock_guard<std::mutex> lock(_queueMutex);
        _running = false;
        }
        // Notify all waiting threads to wake up and exit
        _queueCondition.notify_all();
        MainWindow::log("InteractiveChatService is stopped." , LogLevel::LOG_INFO);
    }
}
