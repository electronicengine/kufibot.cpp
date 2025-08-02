#include "gesture_performer_service.h"

#include "tui_service.h"
#include "../logger.h"
#include "../operators/json_parser_operator.h"
#include "../controllers/controller_data_structures.h"

GesturePerformerService* GesturePerformerService::_instance = nullptr;


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
        _idlePosition
    );

    INFO("Setting robot to idle position...");
    setIdlePosition();

    SpeechPerformingOperator::get_instance()->loadModel();
    SpeechRecognizingOperator::get_instance()->load_model();

    subscribe_to_service(InteractiveChatService::get_instance());
    subscribe_to_service(TuiService::get_instance());

    INFO("Entering the gesture performing loop...");
    while (_running) {
        std::unique_lock<std::mutex> lock(_llmResponseQueueMutex);

        _condVar.wait(lock, [this]() {
            return !_llmResponseQueue.empty() || !_running;
        });

        while (!_llmResponseQueue.empty() && !_gestureWorking) {
            LLMResponseData response = _llmResponseQueue.front();
            make_mimic(response);
            _llmResponseQueue.pop();
        }
    }
}


void GesturePerformerService::subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data) {

    std::lock_guard<std::mutex> lock(_dataMutex);

    switch (type) {
        case MessageType::LLMResponse: {
            if (data) {
                LLMResponseData llmResponse = *static_cast<LLMResponseData*>(data.get());
                INFO("LLM Response: {}", llmResponse.sentence);
                _llmResponseQueue.push(llmResponse);
                _condVar.notify_one(); // Notify the processing thread
            }
            break;
        }
        case MessageType::RecognizedGesture:{
            if (data) {
                std::string face_gesture = static_cast<RecognizedGestureData*>(data.get())->faceGesture;
                std::vector<int> face_landmark = static_cast<RecognizedGestureData*>(data.get())->faceLandmark;
                std::string hand_gesture = static_cast<RecognizedGestureData*>(data.get())->handGesture;
                std::vector<int> hand_landmark = static_cast<RecognizedGestureData*>(data.get())->handLandmark;
            }
            break;
        }
        default:
            break;
    }
}



void GesturePerformerService::control_motion(ServoMotorJoint joint, int angle) {
    // This is your existing motor control function
    // Implementation depends on your hardware interface
    DEBUG("Moving joint {} to angle {}", Servo_Motor_Joint_Names.at(joint).c_str(), angle);

    // Example implementation - replace with your actual motor control code
    // motor_controller->move_servo(joint, angle);
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
    DEBUG("Setting robot to idle position");
    for (const auto& [joint, state] : _idlePosition) {
        int angle = getAngleForJointState(joint, state);
        control_motion(joint, angle);
    }
}

void GesturePerformerService::executeJointPositions(const std::map<ServoMotorJoint, GestureJointState>& positions) {
    std::unique_ptr<MessageData> data = std::make_unique<ControlData>();
    std::map<ServoMotorJoint, uint8_t> jointAngles;
    DEBUG("Executing Joint Positions: ");
    for (const auto& [joint, state] : positions) {
        jointAngles[joint] = getAngleForJointState(joint, state);
        DEBUG("{}: {}", Servo_Motor_Joint_Names.at(joint), jointAngles[joint]);
    }

    static_cast<ControlData*>(data.get())->jointAngles.emplace();
    static_cast<ControlData*>(data.get())->jointAngles.value() = jointAngles;

    //publish(MessageType::ControlData, data);
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
    WARNING("Executing emotional motion: {}", motion.name);
    executeMotionSequence(motion.joints, motion.sequence, motion.duration);

}

void GesturePerformerService::executeReactionalMotion(ReactionType reactionType) {

    auto it = _reactionalMotions.find(reactionType);
    if (it == _reactionalMotions.end()) {
        WARNING("Reactional motion not found: {}");
        return;
    }

    const ReactionalMotion& motion = it->second;
    WARNING("Executing reactional motion: {}", motion.name);

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

void GesturePerformerService::make_mimic(const LLMResponseData &llm_response) {
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

