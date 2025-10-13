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

#include "../logger.h"
#include "../operators/json_parser_operator.h"
#include "gesture_performer_service.h"
#include "gesture_recognizer_service.h"
#include "interactive_chat_service.h"
#include "robot_controller_service.h"

constexpr int MAX_ERROR = 300;
constexpr int MIN_MAGNITUDE = 1;
constexpr int MAX_MAGNITUDE = 4;
constexpr int TALKING_TRESH_COUNT = 500;

LandmarkTrackerService* LandmarkTrackerService::_instance = nullptr;

LandmarkTrackerService * LandmarkTrackerService::get_instance() {
    if (_instance == nullptr) {
        _instance = new LandmarkTrackerService();
    }
    return _instance;
}


LandmarkTrackerService::LandmarkTrackerService() : Service("LandmarkTrackingService") {
    _faceFound = false;
    _handFound = false;
    _gesturePerformanceCompleted = false;
    _errorTreshold = 50;
    _talkingTimeCounter = 0;
}


LandmarkTrackerService::~LandmarkTrackerService() {
}


void LandmarkTrackerService::service_function() {
    //add thread sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    subscribe_to_service(GestureRecognizerService::get_instance());
    subscribe_to_service(InteractiveChatService::get_instance());
    subscribe_to_service(RobotControllerService::get_instance());
    subscribe_to_service(GesturePerformerService::get_instance());

    _jointLimits = JsonParserOperator::getJointLimits("/usr/local/etc/joint_angles.json");
    INFO("Test _jointLimits[ServoMotorJoint::leftArm][GestureJointState::fulldown].angle = {}", _jointLimits[ServoMotorJoint::leftArm][GestureJointState::fulldown].angle);

    INFO("Entering the tracking loop...");

    while (_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        _dataMutex.lock();
            std::vector<int> handBbox = _recognizedGestureData.handBbox;
            _recognizedGestureData.handBbox.clear();
            std::map<ServoMotorJoint, uint8_t> currentJointAngles;
            if (_sensorData.currentJointAngles.has_value()) {
                currentJointAngles = _sensorData.currentJointAngles.value();
                _sensorData.currentJointAngles.reset();
            }
            std::vector<int> faceLandMarks = _recognizedGestureData.faceLandmark;
        _dataMutex.unlock();

        if (!faceLandMarks.empty()) {
            int sumX = 0;
            int sumY = 0;
            int eyePointCount = 0;

            for (size_t i = 0; i < faceLandMarks.size(); i += 3) {
                int id = faceLandMarks[i];
                int cx = faceLandMarks[i + 1];
                int cy = faceLandMarks[i + 2];

                // eye points
                if (id == 33 || id == 133 || id == 362 || id == 263) {
                    sumX += cx;
                    sumY += cy;
                    eyePointCount++;
                }
            }

            if (eyePointCount > 0) {
                int centerX = sumX / eyePointCount;
                int centerY = sumY / eyePointCount;

                ErrorVector errorVector = calculateErrorVector(centerX, centerY);
                if (errorVector.magnitude > _errorTreshold) {
                    if (_errorTreshold == 250) {
                        _errorTreshold = 50;
                    }

                    //scale down it between  1 and 5
                    int magnitude = MIN_MAGNITUDE +
                        (errorVector.magnitude - _errorTreshold) * (MAX_MAGNITUDE - MIN_MAGNITUDE) / (MAX_ERROR - _errorTreshold);

                    // Clamp to [1, 5]
                    if (magnitude < MIN_MAGNITUDE) magnitude = MIN_MAGNITUDE;
                    if (magnitude > MAX_MAGNITUDE) magnitude = MAX_MAGNITUDE;

                    controlHead(errorVector.angle, magnitude);
                    if (!currentJointAngles.empty()) {
                        GesturePerformerService::idleJointPositions = currentJointAngles;
                    }
                } else {
                    if (_talkingTimeCounter >= TALKING_TRESH_COUNT || _talkingTimeCounter == 0) {
                        sayHello();
                        _talkingTimeCounter = 1;
                    }
                    INFO("count: {} ", _talkingTimeCounter);
                    ++_talkingTimeCounter;
                }
            }
        } else if (!handBbox.empty() && handBbox.size() == 4) {

            int centerX = (handBbox[0] + handBbox[2]) / 2;
            int centerY = (handBbox[1] + handBbox[3]) / 2;

            ErrorVector errorVector = calculateErrorVector(centerX, centerY);
            if (errorVector.magnitude > _errorTreshold) {
                //scale down it between  1 and 5
                int magnitude = MIN_MAGNITUDE +
                    (errorVector.magnitude - _errorTreshold) * (MAX_MAGNITUDE - MIN_MAGNITUDE) / (MAX_ERROR - _errorTreshold);

                // Clamp to [1, 5]
                if (magnitude < MIN_MAGNITUDE) magnitude = MIN_MAGNITUDE;
                if (magnitude > MAX_MAGNITUDE) magnitude = MAX_MAGNITUDE;

                controlHead(errorVector.angle, magnitude);
                if (!currentJointAngles.empty()) {
                    GesturePerformerService::idleJointPositions = currentJointAngles;
                }
            }

        } else {
            //searchFace();
        }
    }
}

ErrorVector LandmarkTrackerService::calculateErrorVector(int centerX, int centerY) {
    ErrorVector errorVector;

    int errorX = centerX - 320; // Target X is 320
    int errorY = centerY - 240; // Target Y is 240

    // Calculate magnitude of error vector
    double magnitude = sqrt(errorX * errorX + errorY * errorY);
    INFO("Error Magnitude: {}",magnitude);


    double angle = atan2(-errorY, errorX) * 180.0 / M_PI;

    // Normalize angle to 0-360 range
    if (angle < 0) {
        angle += 360;
    }

    errorVector.angle = angle;
    errorVector.magnitude = magnitude;

    return errorVector;
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
            break;
        }

        case MessageType::SensorData: {
            if (data) {
                _sensorData = *static_cast<SensorData*>(data.get());
            }
            break;
        }

        case MessageType::GesturePerformanceCompleted : {
            _gesturePerformanceCompleted = true;
            _condVar.notify_one();
            break;
        }

        default:
            break;
    }

}


void LandmarkTrackerService::controlHead(int angle, int magnitude) {
    std::unique_ptr<MessageData> data = std::make_unique<ControlData>();
    data->source = SourceService::landmarkTrackerService;
    static_cast<ControlData *>(data.get())->headJoystick.emplace();

    auto &joyStick = static_cast<ControlData *>(data.get())->headJoystick.value();
    joyStick.angle = angle;
    joyStick.strength = magnitude;

    publish(MessageType::ControlData, data);
}

void LandmarkTrackerService::sayHello() {

    // if (_faceFound)
    //     return;
    //
    // _faceFound = true;

    std::unique_ptr<MessageData> data = std::make_unique<LLMResponseData>();
    static_cast<LLMResponseData *>(data.get())->sentence = "Hey Buddy.";
    static_cast<LLMResponseData *>(data.get())->reactionSimilarity = 0.9;
    static_cast<LLMResponseData *>(data.get())->emotionSimilarity = 0;
    static_cast<LLMResponseData *>(data.get())->reactionalGesture.reaction = ReactionType::greeting;

    ERROR("Day Hello!!");
    _errorTreshold = 250;
    _gesturePerformanceCompleted = false;

    publish(MessageType::LLMResponse, data);

    std::unique_lock<std::mutex> lock(_dataMutex);
    _condVar.wait(lock, [this] { return _gesturePerformanceCompleted.load(); });

}
