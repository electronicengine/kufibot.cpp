
#include "landmark_tracker_service.h"

#include "gesture_recognizer_service.h"
#include "robot_controller_service.h"
#include "interactive_chat_service.h"
#include "../logger.h"

LandmarkTrackerService* LandmarkTrackerService::_instance = nullptr;

LandmarkTrackerService * LandmarkTrackerService::get_instance() {
    if (_instance == nullptr) {
        _instance = new LandmarkTrackerService();
    }
    return _instance;
}


LandmarkTrackerService::LandmarkTrackerService() : Service("LandmarkTrackingService") {

}

LandmarkTrackerService::~LandmarkTrackerService() {
}


void LandmarkTrackerService::service_function() {

    subscribe_to_service(GestureRecognizerService::get_instance());
    subscribe_to_service(InteractiveChatService::get_instance());

    INFO("Entering the tracking loop...");

    while (_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

}

void LandmarkTrackerService::searchTheFace() {

}

void LandmarkTrackerService::subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData> &data) {
    std::lock_guard<std::mutex> lock(_dataMutex);

    switch (type) {
        case MessageType::LLMResponse: {
            if (data) {
                _llmResponseData = *static_cast<LLMResponseData*>(data.get());

            }
            break;
        }
        case MessageType::RecognizedGesture : {
            if (data) {
                _recognizedGestureData = *static_cast<RecognizedGestureData*>(data.get());
            }
        }

        default:
            break;
    }

}
