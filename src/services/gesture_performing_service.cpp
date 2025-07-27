#include "gesture_performing_service.h"

#include "tui_service.h"
#include "../logger.h"
#include "../operators/json_parser_operator.h"

GesturePerformingService* GesturePerformingService::_instance = nullptr;


GesturePerformingService *GesturePerformingService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new GesturePerformingService();
    }
    return _instance;
}


GesturePerformingService::GesturePerformingService() : Service("GesturePerformingService") {

    // Load models
    SpeechPerformingOperator::get_instance()->loadModel();
    SpeechRecognizingOperator::get_instance()->load_model();
    JsonParserOperator::get_instance()->loadGestureJointAnglesFromJson("/usr/local/etc/joint_angles.json", _jointGesturePositionList);

    _gestureWorking = false;

}


GesturePerformingService::~GesturePerformingService()
{

}


void GesturePerformingService::greeter()
{
    
    INFO("greeter !");
     std::map<std::string, int> jointAngles = {{"right_arm", 20}, {"left_arm", 170}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 170},{"eye_left", 20}};

    for(int i=0; i<2; i++){
        jointAngles = {{"right_arm", 20}, {"left_arm", 120}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 170},{"eye_left", 20}};

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        jointAngles = {{"right_arm", 20}, {"left_arm", 150}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 170},{"eye_left", 20}};

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

     }

}

void GesturePerformingService::knowledgeable()
{
    INFO("knowledgeable !");
    std::map<std::string, int> jointAngles = {{"right_arm", 20}, {"left_arm", 170}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 130},{"eye_left", 0}};

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void GesturePerformingService::optimistic()
{
    INFO("optimistic !");
     std::map<std::string, int> jointAngles = {{"right_arm", 40}, {"left_arm", 140}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 170},{"eye_left", 20}};

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void GesturePerformingService::pessimistic()
{
    INFO("pessimistic !");
    std::map<std::string, int> jointAngles = {{"right_arm", 20}, {"left_arm", 170}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 130},{"eye_left", 50}};

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void GesturePerformingService::curious()
{
    INFO("curious !");
    std::map<std::string, int> jointAngles = {{"right_arm", 20}, {"left_arm", 170}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 170},{"eye_left", 20}};

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    jointAngles = {{"right_arm", 20}, {"left_arm", 150}, {"neck_down", 78},{"neck_up", 15}, {"neck_right", 90}, {"eye_right", 180},{"eye_left", 0}};

}

void GesturePerformingService::recognized_gesture(const std::string &_faceGesture, std::vector<int> _faceLandMark,
    const std::string &_handGesture, std::vector<int> _handLandmark) {

    //to DO take care the recognized gesture and publish the Llama Query


}

void GesturePerformingService::llm_response(const std::string &response) {

    std::string gesture = response;
    //to DO  seperate gesture info form llama string

    if (gesture.find("selamlayan") != std::string::npos) {
        greeter();
    } else if (gesture.find("bilgili") != std::string::npos) {
        knowledgeable();
    } else if (gesture.find("iyimser") != std::string::npos) {
        optimistic();
    } else if (gesture.find("kötümser") != std::string::npos) {
        pessimistic();
    } else if (gesture.find("meraklı") != std::string::npos) {
        curious();
    } else {
        ERROR("Unknown gesture:");
    }
}

void GesturePerformingService::service_function()
{
    INFO("GesturePerformingService::_interactiveChatService::subscribe");

    subscribe_to_service(InteractiveChatService::get_instance());
    subscribe_to_service(TuiService::get_instance());

}

void GesturePerformingService::subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data) {

    std::lock_guard<std::mutex> lock(_dataMutex);

    switch (type) {
        case MessageType::LLMResponse: {
            if (data) {
                // std::string msg = static_cast<LLMResponseData*>(data.get())->response;
                // llm_response(msg);
            }
            break;
        }
        case MessageType::RecognizedGesture:{
            if (data) {
                std::string face_gesture = static_cast<RecognizedGestureData*>(data.get())->faceGesture;
                std::vector<int> face_landmark = static_cast<RecognizedGestureData*>(data.get())->faceLandmark;
                std::string hand_gesture = static_cast<RecognizedGestureData*>(data.get())->handGesture;
                std::vector<int> hand_landmark = static_cast<RecognizedGestureData*>(data.get())->handLandmark;

                recognized_gesture(face_gesture, face_landmark, hand_gesture, hand_landmark);
            }
            break;
        }
        default:
            WARNING("{} subcribed_data_receive unknown message type!", get_service_name());
            break;
    }
}

