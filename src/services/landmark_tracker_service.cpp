/*
* This file is part of Kufibot.
 *
 * Kufibot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kufibot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kufibot. If not, see <https://www.gnu.org/licenses/>.
 */

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
