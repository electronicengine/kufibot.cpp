#include "interactive_chat_service.h"

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
        std::cout << "message: " << message << std::endl;
        if(message.find("kofi") != std::string::npos){
            _speechProcessController->speakText("dinliyorum");

            _speechRecognitionController->start_listen();
            std::string message = _speechRecognitionController->get_message();  
            _speechRecognitionController->stop_listen();
            Json json_msg = Json::parse(message);
            message = json_msg["text"];
            std::cout << "execute_gemini: " << message << std::endl;
            //  message = translate("tr", "en", message);
            // std::cout << "Translated Message: " << message << std::endl;
            std::string result = _curlController->execute_gemini(message);
            // result = translate("en", "tr", result);
            std::cout << "result" << result << std::endl;
            std::vector<std::string> sentences = splitSentences(result);
            for (const auto& sentence : sentences) {
                std::cout << "sentence: " << sentence << std::endl;
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

    }else if(message.find("tanım") != std::string::npos){
        _speechProcessController->speakText("Şu an inceliyorum...");
            cv::Mat photo = _videoStreamService->take_snap_shot();
            if (cv::imwrite("../gemini/image.jpg", photo)) {
                std::cout << "snap shot" << std::endl;
            } else {
                std::cerr << "Error: Could not save the photo." << std::endl;
            }

            std::string result = _executionController->execute(ExecutionType::imageQuery, "Fotoğrafı Türkçe olarak ve bir resim olduğunu belirtmeden açıkla");
            std::vector<std::string> sentences = splitSentences(result);
            for (const auto& sentence : sentences) {
                std::cout << "sentence: " << sentence << std::endl;
                _speechProcessController->speakText(sentence);
            }

    }else{
        // std::string result = _curlController->execute_gemini(message);
        std::string result = _executionController->execute(ExecutionType::query, message);
        std::cout << "result" << result << std::endl;
        std::vector<std::string> sentences = splitSentences(result);
        for (const auto& sentence : sentences) {
            std::cout << "sentence: " << sentence << std::endl;
            _speechProcessController->speakText(sentence);
        }
    }
}

void InteractiveChatService::update_web_socket_message(websocketpp::connection_hdl hdl, const std::string &msg)
{
    if(msg == "on_open"){
        std::cout << "new web socket connection extablished" << std::endl;
    }
    else if(msg == "on_close"){
        std::cout << "web socket connection is closed." << std::endl;
    }
    else{
        Json message = Json::parse(msg);
        if (message.contains("talkie")) {
            std::string incoming_talkie = message["talkie"];
            std::cout << incoming_talkie << std::endl;

            std::thread walkie_thread = std::thread(&InteractiveChatService::walkie_talkie_thread, this, incoming_talkie);
            walkie_thread.detach();

        }
    }
}

void InteractiveChatService::start()
{
    if (!_running) { // Ensure the thread is not already running
        _running = true;
         
        std::cout << "InteractiveChatService::_webSocketService::subscribe" << std::endl;

        _webSocketService->subscribe(this);
        std::cout << "InteractiveChatService is starting..." << std::endl;

    }
}

void InteractiveChatService::stop()
{
    if (_running){
        _running = false;
        std::cout << "InteractiveChatService is stopping..." << std::endl;

        _speechRecognitionController->stop_listen();
        _speechRecognitionController->close();

        if (_serviceThread.joinable()) {
            _serviceThread.join(); 
        }
        std::cout << "InteractiveChatService is stopped." << std::endl;
    }
}
