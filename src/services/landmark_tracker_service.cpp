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
constexpr int REACTION_TIME_START = 430;
constexpr int REACTION_TRESHOLD_COUNT = 500;

LandmarkTrackerService* LandmarkTrackerService::_instance = nullptr;

LandmarkTrackerService * LandmarkTrackerService::get_instance() {
    if (_instance == nullptr) {
        _instance = new LandmarkTrackerService();
    }
    return _instance;
}


LandmarkTrackerService::LandmarkTrackerService() : Service("LandmarkTrackingService") {
    _errorTreshold = 50;
    _reactionEngageTimeout = REACTION_TIME_START;
}
void LandmarkTrackerService::initialize() {

    subscribe_to_service(GestureRecognizerService::get_instance());
    subscribe_to_service(InteractiveChatService::get_instance());
    subscribe_to_service(RobotControllerService::get_instance());
    subscribe_to_service(GesturePerformerService::get_instance());

    _jointLimits = JsonParserOperator::getJointLimits("/usr/local/etc/joint_angles.json");
}


TrackingData LandmarkTrackerService::collectTrackingData() {
    std::lock_guard<std::mutex> lock(_dataMutex);
    TrackingData trackData;

    trackData.recognizedGestureData = _recognizedGestureData;

    if (_sensorData.currentJointAngles.has_value()) {
        trackData.currentJointAngles = _sensorData.currentJointAngles.value();
        _sensorData.currentJointAngles.reset();
    }

    _sensorData.currentJointAngles.reset();

    return trackData;
}


Point2D LandmarkTrackerService::selectTheTarget(const TrackingData &trackData) {
    // Priority: Face > Hand
    if (!trackData.recognizedGestureData.faceLandmarks.empty() && trackData.recognizedGestureData.faceLandmarks.size() > 3) {
        _lastKnownTarget.setValue(trackData.getFaceCenter());
        return _lastKnownTarget.getValue();
    } else if (trackData.recognizedGestureData.handBbox.valid) {
       _lastKnownTarget.setValue(trackData.getHandCenter());
        return _lastKnownTarget.getValue();
    } else {
        return _lastKnownTarget.getValue();
    }
}

TrackState LandmarkTrackerService::getTrackingState(const PolarVector &errorVector) {

    if (errorVector.magnitude > _errorTreshold) {
        return TrackState::tracking;
    } else if (errorVector.angle == 0 && errorVector.magnitude == 0) {
        return TrackState::idle;
    } else{
        if (_reactionEngageTimeout >= REACTION_TRESHOLD_COUNT) {
            _reactionEngageTimeout = 1;
            return TrackState::engaging_reaction;
        } else {
            ++_reactionEngageTimeout;
            return TrackState::idle;
        }
    }
}

int LandmarkTrackerService::calculateControlMagnitude(const PolarVector &errorVector) {
    //scale down it between  1 and 5
    int magnitude = MIN_MAGNITUDE +
        (errorVector.magnitude - _errorTreshold) * (MAX_MAGNITUDE - MIN_MAGNITUDE) / (MAX_ERROR - _errorTreshold);

    // Clamp to [1, 5]
    if (magnitude < MIN_MAGNITUDE) magnitude = MIN_MAGNITUDE;
    if (magnitude > MAX_MAGNITUDE) magnitude = MAX_MAGNITUDE;

    // INFO("error mag: {} - {}", magnitude, errorVector.magnitude);

    return magnitude;
}


LandmarkTrackerService::~LandmarkTrackerService() {
}


void LandmarkTrackerService::service_function() {

    initialize();

    INFO("Entering the tracking loop...");

    while (_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        TrackingData trackingData = collectTrackingData();
        Point2D target = selectTheTarget(trackingData);
        PolarVector errorVector = calculateErrorVector(target);

        TrackState trackState = getTrackingState(errorVector);

        switch (trackState) {
            case TrackState::tracking: {

                int magnitude = calculateControlMagnitude(errorVector);
                controlHead(errorVector.angle, magnitude);

                if (!trackingData.currentJointAngles.empty()) {
                    GesturePerformerService::idleJointPositions = trackingData.currentJointAngles;
                }

                break;
            }

            case TrackState::engaging_reaction: {
                engageReaction(trackingData);
                break;
            }

            case TrackState::idle: {

                break;
            }

            default:
                break;
        }
    }
}


PolarVector LandmarkTrackerService::calculateErrorVector(const Point2D &target) {
    PolarVector errorVector;
    if (target.x == 0 && target.y == 0)
        return PolarVector{0,0};

    int errorX = target.x - 320; // Target X is 320
    int errorY = target.y - 240; // Target Y is 240

    // Calculate magnitude of error vector
    double magnitude = sqrt(errorX * errorX + errorY * errorY);

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

    switch (type) {
        case MessageType::LLMResponse: {
            if (data) {
                std::lock_guard<std::mutex> lock(_dataMutex);

                _llmResponseData = *static_cast<LLMResponseData*>(data.get());
            }
            break;
        }

        case MessageType::RecognizedGesture : {
            if (data) {
                std::lock_guard<std::mutex> lock(_dataMutex);

                _recognizedGestureData = *static_cast<RecognizedGestureData*>(data.get());
            }
            break;
        }

        case MessageType::SensorData: {
            if (data) {
                std::lock_guard<std::mutex> lock(_dataMutex);

                _sensorData = *static_cast<SensorData*>(data.get());
            }
            break;
        }

        case MessageType::GesturePerformanceCompleted : {
            start();
            break;
        }

        case MessageType::InteractiveChatStarted : {
            stop();
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

void LandmarkTrackerService::engageReaction(TrackingData trackingData) {
    INFO("engageReaction! - faceGesture{}", trackingData.recognizedGestureData.faceEmotion);
    if (trackingData.recognizedGestureData.faceEmotion == "No Face") {
        return;
    }

    std::unique_ptr<MessageData> data = std::make_unique<LLMQueryData>();
    static_cast<LLMQueryData *>(data.get())->query = "You are looking a person. He/She seems " + trackingData.recognizedGestureData.faceEmotion + ". Say something to her/him.";
    publish(MessageType::LLMQuery, data);

}
