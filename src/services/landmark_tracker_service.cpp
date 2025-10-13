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

    //add thread sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(4000));

    subscribe_to_service(GestureRecognizerService::get_instance());
    subscribe_to_service(InteractiveChatService::get_instance());
    subscribe_to_service(RobotControllerService::get_instance());

    _jointLimits = JsonParserOperator::getJointLimits("/usr/local/etc/joint_angles.json");
    INFO("Test _jointLimits[ServoMotorJoint::leftArm][GestureJointState::fulldown].angle = {}", _jointLimits[ServoMotorJoint::leftArm][GestureJointState::fulldown].angle);

    INFO("Entering the tracking loop...");

    while (_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::lock_guard<std::mutex> lock(_dataMutex);

        std::vector<int> handBbox = _recognizedGestureData.handBbox;
        _recognizedGestureData.handBbox.clear();

        if (!_recognizedGestureData.faceLandmark.empty()) {
            int sumX = 0;
            int sumY = 0;
            int eyePointCount = 0;

            for (size_t i = 0; i < _recognizedGestureData.faceLandmark.size(); i += 3) {
                int id = _recognizedGestureData.faceLandmark[i];
                int cx = _recognizedGestureData.faceLandmark[i + 1];
                int cy = _recognizedGestureData.faceLandmark[i + 2];

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
                if (errorVector.magnitude > 50) {
                    controlHead(errorVector.angle);
                    if (_sensorData.currentJointAngles.has_value()) {
                        GesturePerformerService::idleJointPositions = _sensorData.currentJointAngles.value();
                        _sensorData.currentJointAngles.reset();
                    }
                }
            }
        } else if (!handBbox.empty() && handBbox.size() == 4) {

            int centerX = (handBbox[0] + handBbox[2]) / 2;
            int centerY = (handBbox[1] + handBbox[3]) / 2;

            ErrorVector errorVector = calculateErrorVector(centerX, centerY);
            if (errorVector.magnitude > 50) {
                controlHead(errorVector.angle);
                if (_sensorData.currentJointAngles.has_value()) {
                    GesturePerformerService::idleJointPositions = _sensorData.currentJointAngles.value();
                    _sensorData.currentJointAngles.reset();
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

        default:
            break;
    }

}

void LandmarkTrackerService::searchFace() {
    static int searchPhase = 0;

    switch (searchPhase) {
        case 0: {
            controlHead(LEFT);

            if (!_sensorData.currentJointAngles.has_value())
                break;

            auto headPos = _sensorData.currentJointAngles.value()[ServoMotorJoint::headLeftRight];
            INFO("searchFace left degree: {} - {}", headPos,
                 _jointLimits[ServoMotorJoint::headLeftRight][GestureJointState::fulldown].angle);

            if (headPos >= _jointLimits[ServoMotorJoint::headLeftRight][GestureJointState::fulldown].angle)
                searchPhase = 1;

            break;
        }

        case 1: {
            controlHead(RIGHT);

            if (!_sensorData.currentJointAngles.has_value())
                break;

            auto headPos = _sensorData.currentJointAngles.value()[ServoMotorJoint::headLeftRight];
            INFO("searchFace right degree: {} - {}", headPos,
                 _jointLimits[ServoMotorJoint::headLeftRight][GestureJointState::middle].angle);


            if (headPos <= _jointLimits[ServoMotorJoint::headLeftRight][GestureJointState::middle].angle)
                searchPhase = 2;

            break;
        }

        case 2: {
            controlHead(UP);

            if (!_sensorData.currentJointAngles.has_value())
                break;

            auto headPos = _sensorData.currentJointAngles.value()[ServoMotorJoint::neck];
            INFO("searchFace up degree: {} - {}", headPos,
                 _jointLimits[ServoMotorJoint::neck][GestureJointState::fullup].angle);

            if (headPos >= _jointLimits[ServoMotorJoint::neck][GestureJointState::fullup].angle)
                searchPhase = 3;
            break;
        }

        case 3: {
            controlHead(DOWN);

            if (!_sensorData.currentJointAngles.has_value())
                break;

            auto headPos = _sensorData.currentJointAngles.value()[ServoMotorJoint::neck];
            INFO("searchFace left degree: {} - {}", headPos,
                 _jointLimits[ServoMotorJoint::neck][GestureJointState::middle].angle);

            if (headPos <= _jointLimits[ServoMotorJoint::neck][GestureJointState::middle].angle)
                searchPhase = 0;

            break;
        }
    }
}



void LandmarkTrackerService::controlHead(int angle) {
    std::unique_ptr<MessageData> data = std::make_unique<ControlData>();
    data->source = SourceService::landmarkTrackerService;
    static_cast<ControlData *>(data.get())->headJoystick.emplace();

    auto &joyStick = static_cast<ControlData *>(data.get())->headJoystick.value();
    joyStick.angle = angle;
    joyStick.strength = 100;

    publish(MessageType::ControlData, data);
}
