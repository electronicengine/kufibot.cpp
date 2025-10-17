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

#include "gesture_performer_service.h"

#include "gesture_recognizer_service.h"
#include "landmark_tracker_service.h"
#include "tui_service.h"
#include "video_stream_service.h"
#include "../logger.h"
#include "../operators/json_parser_operator.h"
#include "../controllers/controller_data_structures.h"

GesturePerformerService* GesturePerformerService::_instance = nullptr;
std::map<ServoMotorJoint, uint8_t>  GesturePerformerService::idleJointPositions= Default_Joint_Angles;

GesturePerformerService *GesturePerformerService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new GesturePerformerService();
    }
    return _instance;
}


GesturePerformerService::GesturePerformerService() : Service("GesturePerformerService") {
        _gestureWorking = false;
}


GesturePerformerService::~GesturePerformerService()
{

}


void GesturePerformerService::service_function()
{
    INFO("loading GestureJointAngles From json...");
    JsonParserOperator::get_instance()->loadGestureJointAnglesFromJson(
        "/usr/local/etc/joint_angles.json",
        jointGesturePositionList
    );

    INFO("Loading motion definitions from json...");
    JsonParserOperator::get_instance()->loadMotionsFromFile(
        "/usr/local/etc/motion_definitions.json",
        _emotionalMotions,
        _reactionalMotions,
        _directiveMotions,
        _idlePositions
    );

    INFO("Setting robot to idle position...");
   // setIdlePosition();

    SpeechPerformingOperator::get_instance()->loadModel();

    subscribe_to_service(InteractiveChatService::get_instance());
    subscribe_to_service(TuiService::get_instance());
    subscribe_to_service(LandmarkTrackerService::get_instance());

    INFO("Entering the gesture performing loop...");
    while (_running) {
        std::unique_lock<std::mutex> lock(_llmResponseQueueMutex);


        _condVar.wait(lock, [this]() {
            if (_llmResponseQueue.empty()) {
                WARNING("Gesture Performance is finished!");
                publish(MessageType::GesturePerformanceCompleted);
            }

            return !_llmResponseQueue.empty();
        });

        while (!_llmResponseQueue.empty() && !_gestureWorking) {
            LLMResponseData response = _llmResponseQueue.front();

            std::thread speak = std::thread(&GesturePerformerService::speakText, this, response.sentence);
            speak.detach();
            makeMimic(response);

            _llmResponseQueue.pop();
            while (_speaking); // Wait for the speech to finish
        }
    }

}


void GesturePerformerService::subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data) {

    std::lock_guard<std::mutex> lock(_dataMutex);

    switch (type) {
        case MessageType::LLMResponse: {
            if (data) {
                LLMResponseData llmResponse = *static_cast<LLMResponseData*>(data.get());
                _llmResponseQueue.push(llmResponse);
                _condVar.notify_one(); // Notify the processing thread
            }
            break;
        }

        default:
            break;
    }
}


int GesturePerformerService::getAngleForJointState(ServoMotorJoint joint, GestureJointState state) {
    auto jointIt = jointGesturePositionList.find(joint);
    if (jointIt != jointGesturePositionList.end()) {
        auto stateIt = jointIt->second.find(state);
        if (stateIt != jointIt->second.end()) {
            return stateIt->second.angle;
        }
    }

    WARNING("Angle not found for joint  state ");
    return 90; // Default safe angle
}

void GesturePerformerService::setIdlePosition() {
    std::unique_ptr<MessageData> data = std::make_unique<ControlData>();
    data->source = SourceService::gesturePerformerService;

    static_cast<ControlData*>(data.get())->jointAngles.emplace();
    static_cast<ControlData*>(data.get())->jointAngles.value() = idleJointPositions;
    _currentPositions = idleJointPositions;

    publish(MessageType::ControlData, data);
}

void GesturePerformerService::executeJointPositions(const std::map<ServoMotorJoint, GestureJointState>& positions) {
    std::unique_ptr<MessageData> data = std::make_unique<ControlData>();
    data->source = SourceService::gesturePerformerService;

    std::map<ServoMotorJoint, uint8_t> jointAngles = _currentPositions ;
    for (const auto& [joint, state] : positions) {
        jointAngles[joint] = getAngleForJointState(joint, state);
    }

    static_cast<ControlData*>(data.get())->jointAngles.emplace();
    static_cast<ControlData*>(data.get())->jointAngles.value() = jointAngles;

    publish(MessageType::ControlData, data);
    _currentPositions = jointAngles;
}

void GesturePerformerService::stopCurrentMotion() {
    _gestureWorking = false;
}

void GesturePerformerService::executeEmotionalMotion(EmotionType emotionType) {

    auto it = _emotionalMotions.find(emotionType);
    if (it == _emotionalMotions.end()) {
        WARNING("Emotional motion not found: ");
        return;
    }

    const EmotionalMotion& motion = it->second;
    executeMotionSequence(motion.joints, motion.sequence, motion.duration);

}

void GesturePerformerService::executeReactionalMotion(ReactionType reactionType) {

    auto it = _reactionalMotions.find(reactionType);
    if (it == _reactionalMotions.end()) {
        WARNING("Reactional motion not found: {}");
        return;
    }

    const ReactionalMotion& motion = it->second;

    executeMotionSequence(motion.joints, motion.sequence, motion.duration);

}

void GesturePerformerService::executeDirectiveMotion(DirectiveType directiveType) {
    //toDO
}

void GesturePerformerService::executeMotionSequence(const std::map<ServoMotorJoint, GestureJointState>& baseJoints,
                                                    const std::vector<MotionSequenceItem>& sequence, int totalDuration)
{

    auto startTime = std::chrono::steady_clock::now();

    executeJointPositions(baseJoints);

    // If no sequence, just hold the base position for the duration
    if (sequence.empty()) {
        TRACE(" just hold the base position for the duration");
        std::this_thread::sleep_for(std::chrono::milliseconds(totalDuration));
        return;
    }

    // Execute sequence
    size_t currentSequenceIndex = 0;

    while (currentSequenceIndex < sequence.size() && _gestureWorking) {
        auto currentTime = std::chrono::steady_clock::now();
        int elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            currentTime - startTime).count();

        const MotionSequenceItem& currentItem = sequence[currentSequenceIndex];

        if (elapsedMs >= currentItem.time) {
            // Execute this sequence item
            INFO("Executing sequence: {}", currentSequenceIndex);
            executeJointPositions(currentItem.joints);
            currentSequenceIndex++;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    // Wait for the remaining duration
    auto currentTime = std::chrono::steady_clock::now();
    int elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        currentTime - startTime).count();

    if (elapsedMs < totalDuration && _gestureWorking) {
        INFO("Waiting for the remaining duration of gesture: {} ms", totalDuration - elapsedMs);
        std::this_thread::sleep_for(std::chrono::milliseconds(totalDuration - elapsedMs));
    }

    // Return to idle position after motion completes
    if (_gestureWorking) {
        setIdlePosition();
    }
}

void GesturePerformerService::makeMimic(const LLMResponseData &llm_response) {
    if (_gestureWorking) {
        INFO("Gesture already in progress, skipping new gesture");
        return;
    }

    _gestureWorking = true;

    if (llm_response.reactionSimilarity > llm_response.emotionSimilarity) {
        executeReactionalMotion(llm_response.reactionalGesture.reaction);
    } else {
        executeEmotionalMotion(llm_response.emotionalGesture.emotion);
    }

    _gestureWorking = false;

}

void GesturePerformerService::speakText(const std::string &text) {
    _speaking = true;
    SpeechPerformingOperator::get_instance()->speakText(text);
    _speaking = false;
}



