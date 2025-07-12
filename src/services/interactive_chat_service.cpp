#include "interactive_chat_service.h"
#include "../logger.h"
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
    _videoStreamService = VideoStreamService::get_instance();

    Logger::info("Speech Process Model loading...");
    _speechProcessController->loadModel();
    Logger::info("Speech Recognition Model loading...");
    _speechRecognitionController->load_model();
    Logger::info("Llama Chat Model loading...");
    bool ret = _llamaChatController.loadChatModel();
    if (!ret) {
        Logger::error("Llama Chat Model loading failed!");
    }

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

    // if (_queryRunning) {
    //     return false;
    // } else {

        // std::function<void(const ollama::response&)> response_callback =
        //     [this, onReceiveLlamaResponse](const ollama::response& response) {
        //
        //         std::string partial = response.as_simple_string();
        //
        //         _responseStr += partial;
        //
        //         if (partial.find("#") != std::string::npos) {
        //
        //             push_speak_string(_responseStr);
        //             onReceiveLlamaResponse(_responseStr);
        //             _responseStr.clear();
        //         }
        //
        //         if (response.as_json()["done"]==true) { _queryRunning = false; _responseStr.clear();}
        //     };
        _llamaChatController.setCallBackFunction(onReceiveLlamaResponse);
        _llamaChatController.chat(message);

        //_queryRunning = true;
        return true;
    // }
}


bool InteractiveChatService::load_model(const LlamaOptions &llamaOptions)
{
    _llamaChatController.setCallBackFunction([this](const std::string& text) {

    });

    return _llamaChatController.loadChatModel(llamaOptions.llamaChatModelPath);

}

void InteractiveChatService::service_update_function()
{
    while (_running) {
        std::string text;

        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            _queueCondition.wait(lock, [this] {
                return !_stringQueue.empty() || !_running;
            });
            if (!_running && _stringQueue.empty()) {
                break;
            }
            text = _stringQueue.front();
            _stringQueue.pop_front();
        }


        if (!text.empty()) {
            std::string sentence;
            std::string gesture;
            size_t start = text.find('/') + 1;
            size_t end = text.find('#');    

            if (start != std::string::npos && end != std::string::npos && start < end) {
                sentence = text.substr(0, start -1);
                gesture = text.substr(start, end - start); 
            }
    
            update_llama_gesture(gesture);
            _speechProcessController->speakText(sentence);
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
    (void) hdl;

    if(msg == "on_open"){
        Logger::trace( "new web socket connection extablished");
    }
    else if(msg == "on_close"){
        Logger::trace( "web socket connection is closed." );
    }
    else{
        Json message = Json::parse(msg);
        if (message.contains("talkie")) {
            std::string incoming_talkie = message["talkie"];
            Logger::trace( "incoming_talkie: {}" + incoming_talkie );

        std::function<void(const ollama::response&)> response_callback =
            [=](const ollama::response& response) {
                Logger::trace( response);
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


        _serviceThread = std::thread(&InteractiveChatService::service_update_function, this);
        Logger::info("InteractiveChatService::_webSocketService::subscribe");
        _webSocketService->subscribe(this);
        Logger::info("InteractiveChatService is starting...");

    }
}

void InteractiveChatService::stop()
{
    if (_running){
        _running = false;
        Logger::info("InteractiveChatService is stopping..." );

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
        Logger::info("InteractiveChatService is stopped."  );
    }
}
