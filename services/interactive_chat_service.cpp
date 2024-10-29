#include "interactive_chat_service.h"

InteractiveChatService* InteractiveChatService::_instance = nullptr;

InteractiveChatService *InteractiveChatService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new InteractiveChatService();
    }
    return _instance;
}


InteractiveChatService::InteractiveChatService(){
    _speechProcessController = SpeechProcessController::get_instance();
    _speechRecognitionController = SpeechRecognitionController::get_instance();
    _robotControllerService = RobotControllerService::get_instance();
    // _curlController = CurlController::get_instance("http://192.168.1.20:11434/api/generate");
    _curlController = CurlController::get_instance("https://generativelanguage.googleapis.com/v1beta/tunedModels/kufi-2165:generateContent?key=AIzaSyAj3z8oiHbljABcdsKRAgO05d7zcNS9Bsw");

}


std::string InteractiveChatService::executeCommand(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);

    if (!pipe) {
        std::cerr << "Failed to run command\n";
        return "";
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}


const std::string InteractiveChatService::translate(const std::string& source, const std::string& target, const std::string& text){

    // Path to your Python virtual environment and script
    std::string venvPath = "/home/kufi/venv/bin/activate";
    std::string scriptPath = "/home/kufi/workspace/translate_test.py";

    // Build the command to activate venv and run the Python script
    std::string command ="trans -b " + source + ":" + target + " \"" + text + "\"";
    // Execute the command and capture the translated text
    std::string translatedText = executeCommand(command);

    return translatedText;
}

std::vector<std::string> InteractiveChatService::splitSentences(std::string text) {

    std::vector<std::string> sentences;
    std::string currentSentence;
    
    for (size_t i = 0; i < text.length(); ++i) {
        currentSentence += text[i]; // Add current character to the sentence
        
        // Check for sentence-ending punctuation
        if (text[i] == '.' || text[i] == '!' || text[i] == '?') {
            sentences.push_back(currentSentence); // Add trimmed sentence to the vector
            currentSentence.clear(); // Clear the current sentence for the next one
        }
    }
    
    // If there's any text left in currentSentence, add it as well
    if (!currentSentence.empty()) {
        sentences.push_back(currentSentence);
    }

    return sentences;

}

void InteractiveChatService::chat_loop()
{


    while(_running){


        std::string message = _speechRecognitionController->get_message();  
        _speechRecognitionController->stop_listen();
        Json json_msg = Json::parse(message);
        message = json_msg["text"];
        std::cout << "message: " << message << std::endl;
        if(message.find("kofi") != std::string::npos){
            std::cout << "dinliyor... " << std::endl;

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

InteractiveChatService::~InteractiveChatService()
{
}

void InteractiveChatService::start()
{
    if (!_running) { // Ensure the thread is not already running
        _running = true;
        std::cout << "InteractiveChatService is starting..." << std::endl;
        std::string command = "source /home/kufi/venv/bin/activate";
        executeCommand(command);
        _speechProcessController->loadModel("../ai.models/trSpeechModel/dfki.onnx",
                        "../ai.models/trSpeechModel/dfki.onnx.json");

        _speechRecognitionController->load_model("../ai.models/trRecognizeModel");
        if(!_speechRecognitionController->open()) {
            return ;
        }

        if (!_speechRecognitionController->start_listen()) {
            _speechRecognitionController->close();
            return ;
        }

        _chat_thread = std::thread(&InteractiveChatService::chat_loop, this);
    }
}

void InteractiveChatService::stop()
{
    if (_running){
        _running = false;
        std::cout << "InteractiveChatService is stopping..." << std::endl;

        _speechRecognitionController->stop_listen();
        _speechRecognitionController->close();

        if (_chat_thread.joinable()) {
            _chat_thread.join(); 
        }
        std::cout << "InteractiveChatService is stopped." << std::endl;
    }
}
