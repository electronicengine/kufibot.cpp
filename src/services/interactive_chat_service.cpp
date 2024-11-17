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

std::vector<std::string> InteractiveChatService::splitSentences(std::string text) {

    std::vector<std::string> sentences;
    std::string currentSentence;
    
    for (size_t i = 0; i < text.length(); ++i) {
        currentSentence += text[i]; 
        
        if (text[i] == '.' || text[i] == '!' || text[i] == '?') {
            sentences.push_back(currentSentence); 
            currentSentence.clear();
        }
    }
    
    if (!currentSentence.empty()) {
        sentences.push_back(currentSentence);
    }

    return sentences;

}


InteractiveChatService::~InteractiveChatService()
{
}

void InteractiveChatService::service_update_function()
{
    while(_running){
        std::string message = _speechRecognitionController->get_message();  
        _speechRecognitionController->stop_listen();
        Json json_msg = Json::parse(message);
        message = json_msg["text"];
        MainWindow::log("message: " + message, LogLevel::LOG_TRACE);

        if(message.find("kofi") != std::string::npos){
            _speechProcessController->speakText("dinliyorum");

            _speechRecognitionController->start_listen();
            std::string message = _speechRecognitionController->get_message();  
            _speechRecognitionController->stop_listen();
            Json json_msg = Json::parse(message);
            message = json_msg["text"];
            MainWindow::log("execute_gemini: "  + message, LogLevel::LOG_TRACE);
            //  message = translate("tr", "en", message);
            std::string result = _curlController->execute_gemini(message);
            // result = translate("en", "tr", result);
            MainWindow::log("result: "  + result, LogLevel::LOG_TRACE);
            std::vector<std::string> sentences = splitSentences(result);
            for (const auto& sentence : sentences) {
                MainWindow::log("sentence: "  + sentence, LogLevel::LOG_TRACE);

                _speechProcessController->speakText(sentence);
            }

            _speechRecognitionController->start_listen();


        }else{
            _speechRecognitionController->start_listen();
        }
    }
}

void InteractiveChatService::walkie_talkie_thread(const std::string& message){

    std::lock_guard<std::mutex> lock(_dataMutex);

    if (message.find("mikrofon") != std::string::npos &&  message.find("başlat") != std::string::npos) {
        _speechProcessController->speakText("mikrofon yayını başlatılıyor.");
        if (!_speechRecognitionController->start_listen()) {
            _speechRecognitionController->close();
            return ;
        }
        _serviceThread = std::thread(&InteractiveChatService::service_update_function, this);

    }else if(message.find("mikrofon") != std::string::npos &&  message.find("durdur") != std::string::npos) {
        _speechProcessController->speakText("mikrofon yayını durduruluyor.");
        _speechRecognitionController->stop_listen();
        if (_serviceThread.joinable()) {
            _serviceThread.join(); 
        }
    }else if (message.find("atölye") != std::string::npos && message.find("git") != std::string::npos) {
        _speechProcessController->speakText("Şu an atolye odasına gidiliyor...");
        MappingService *map_service = MappingService::get_instance();
        map_service->go_to_point(22334, 5677);


    }else if(message.find("tanım") != std::string::npos){
        _speechProcessController->speakText("Şu an inceliyorum...");
            cv::Mat photo = _videoStreamService->take_snap_shot();
            if (cv::imwrite("../gemini/image.jpg", photo)) {
                MainWindow::log( "snap shot" , LogLevel::LOG_INFO);

            } else {
                MainWindow::log("Error: Could not save the photo." , LogLevel::LOG_ERROR);

            }

            std::string result = _executionController->execute(ExecutionType::imageQuery, "Fotoğrafı Türkçe olarak ve bir resim olduğunu belirtmeden açıkla");
            std::vector<std::string> sentences = splitSentences(result);
            for (const auto& sentence : sentences) {
                MainWindow::log(  "sentence: " + sentence , LogLevel::LOG_TRACE);
                _speechProcessController->speakText(sentence);
            }

    }else{
        // std::string result = _curlController->execute_gemini(message);
        std::string result = _executionController->execute(ExecutionType::query, message);
        MainWindow::log(  "result"  + result , LogLevel::LOG_TRACE);

        std::vector<std::string> sentences = splitSentences(result);
        for (const auto& sentence : sentences) {
            MainWindow::log(  "sentence"  + sentence , LogLevel::LOG_TRACE);
            _speechProcessController->speakText(sentence);
        }
    }
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

            std::thread walkie_thread = std::thread(&InteractiveChatService::walkie_talkie_thread, this, incoming_talkie);
            walkie_thread.detach();

        }
    }
}

void InteractiveChatService::start()
{
    if (!_running) { // Ensure the thread is not already running
        _running = true;
         
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
        MainWindow::log("InteractiveChatService is stopped." , LogLevel::LOG_INFO);
    }
}
