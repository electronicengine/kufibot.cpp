#include "gesture_performer_service.h"

#include "tui_service.h"
#include "../logger.h"
#include "../operators/json_parser_operator.h"

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
        emotionalMotions,
        reactionalMotions,
        directiveMotions,
        idlePosition
    );

    INFO("Setting robot to idle position...");
    setIdlePosition();

    SpeechPerformingOperator::get_instance()->loadModel();
    SpeechRecognizingOperator::get_instance()->load_model();

    subscribe_to_service(InteractiveChatService::get_instance());
    subscribe_to_service(TuiService::get_instance());
}

void GesturePerformerService::control_motion(ServoMotorJoint joint, int angle) {
    // This is your existing motor control function
    // Implementation depends on your hardware interface
    INFO("Moving joint {} to angle {}", Servo_Joint_Names.at(joint).c_str(), angle);

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
    INFO("Setting robot to idle position");
    for (const auto& [joint, state] : idlePosition) {
        int angle = getAngleForJointState(joint, state);
        control_motion(joint, angle);
    }
}

void GesturePerformerService::executeJointPositions(const std::map<ServoMotorJoint, GestureJointState>& positions) {
    for (const auto& [joint, state] : positions) {
        int angle = getAngleForJointState(joint, state);
        control_motion(joint, angle);
    }
}

void GesturePerformerService::stopCurrentMotion() {
    std::lock_guard<std::mutex> lock(motionMutex);
    currentMotionActive = false;

    if (motionThread && motionThread->joinable()) {
        motionThread->join();
        delete motionThread;
        motionThread = nullptr;
    }
}

void GesturePerformerService::executeEmotionalMotion(EmotionType emotionType) {
    stopCurrentMotion();

    auto it = emotionalMotions.find(emotionType);
    if (it == emotionalMotions.end()) {
        WARNING("Emotional motion not found: ");
        return;
    }

    const EmotionalMotion& motion = it->second;
    INFO("Executing emotional motion: {}", motion.name);

    std::lock_guard<std::mutex> lock(motionMutex);
    currentMotionActive = true;

    motionThread = new std::thread([this, motion]() {
        executeMotionSequence(motion.joints, motion.sequence, motion.duration);
    });
}

void GesturePerformerService::executeReactionalMotion(ReactionType reactionType) {
    stopCurrentMotion();

    auto it = reactionalMotions.find(reactionType);
    if (it == reactionalMotions.end()) {
        WARNING("Reactional motion not found: {}");
        return;
    }

    const ReactionalMotion& motion = it->second;
    INFO("Executing reactional motion: {}", motion.name);

    std::lock_guard<std::mutex> lock(motionMutex);
    currentMotionActive = true;

    motionThread = new std::thread([this, motion]() {
        executeMotionSequence(motion.joints, motion.sequence, motion.duration);
    });
}

void GesturePerformerService::executeDirectiveMotion(DirectiveType directiveType) {
    stopCurrentMotion();

    auto it = directiveMotions.find(directiveType);
    if (it == directiveMotions.end()) {
        WARNING("Directive motion not found: {}");
        return;
    }

    const DirectiveMotion& motion = it->second;
    INFO("Executing directive motion: {}", motion.name);

    std::lock_guard<std::mutex> lock(motionMutex);
    currentMotionActive = true;

    motionThread = new std::thread([this, motion]() {
        executeMotionSequence(motion.joints, motion.sequence, motion.duration);
    });
}

void GesturePerformerService::executeMotionSequence(
    const std::map<ServoMotorJoint, GestureJointState>& baseJoints,
    const std::vector<MotionSequenceItem>& sequence,
    int totalDuration) {

    auto startTime = std::chrono::steady_clock::now();

    // Set initial base position
    executeJointPositions(baseJoints);

    // If no sequence, just hold the base position for the duration
    if (sequence.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(totalDuration));
        return;
    }

    // Execute sequence
    size_t currentSequenceIndex = 0;

    while (currentSequenceIndex < sequence.size() && currentMotionActive) {
        auto currentTime = std::chrono::steady_clock::now();
        int elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            currentTime - startTime).count();

        const MotionSequenceItem& currentItem = sequence[currentSequenceIndex];

        if (elapsedMs >= currentItem.time) {
            // Execute this sequence item
            executeJointPositions(currentItem.joints);
            currentSequenceIndex++;
        } else {
            // Wait a bit before checking again
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    // Wait for the remaining duration
    auto currentTime = std::chrono::steady_clock::now();
    int elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        currentTime - startTime).count();

    if (elapsedMs < totalDuration && currentMotionActive) {
        std::this_thread::sleep_for(std::chrono::milliseconds(totalDuration - elapsedMs));
    }

    // Return to idle position after motion completes
    if (currentMotionActive) {
        setIdlePosition();
    }

    currentMotionActive = false;
}

void GesturePerformerService::make_mimic(const LLMResponseData &llm_response) {
    if (gestureWorking) {
        INFO("Gesture already in progress, skipping new gesture");
        return;
    }

    gestureWorking = true;

    try {
        if (llm_response.reactionSimilarity > llm_response.emotionSimilarity) {
            // Execute reaction mimic
            INFO("Executing reaction mimic - similarity: {}", llm_response.reactionSimilarity);

            executeReactionalMotion(llm_response.reactionalGesture.reaction);

        } else {
            // Execute emotion mimic
            INFO("Executing emotion mimic - similarity: {}", llm_response.emotionSimilarity);

            // Find the best matching emotion type based on your similarity logic
            EmotionType emotionType = getBestEmotionType(llm_response);
            executeEmotionalMotion(emotionType);
        }

        // Check if there are any directive commands
        DirectiveType directiveType = getBestDirectiveType(llm_response);
        if (directiveType != DirectiveType::go || hasDirectiveCommand(llm_response)) {
            // Wait for current motion to complete, then execute directive
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            executeDirectiveMotion(directiveType);
        }

    } catch (const std::exception& e) {
        ERROR("Error executing mimic: {}", e.what());
    }

    gestureWorking = false;
}

EmotionType GesturePerformerService::getBestEmotionType(const LLMResponseData &llm_response) {
    // Implement your logic to determine the best emotion type
    // This could be based on text analysis, emotion vectors, etc.
    // For now, returning a default - you should implement based on your LLMResponseData structure

    // Example implementation - replace with your actual logic
    if (llm_response.emotionSimilarity > 0.8) {
        return EmotionType::happy;
    } else if (llm_response.emotionSimilarity > 0.6) {
        return EmotionType::curious;
    } else if (llm_response.emotionSimilarity > 0.4) {
        return EmotionType::serious;
    } else {
        return EmotionType::worried;
    }
}

ReactionType GesturePerformerService::getBestReactionType(const LLMResponseData &llm_response) {
    // Implement your logic to determine the best reaction type
    // Example implementation - replace with your actual logic

    if (llm_response.reactionSimilarity > 0.8) {
        return ReactionType::agreeing;
    } else if (llm_response.reactionSimilarity > 0.6) {
        return ReactionType::talking;
    } else if (llm_response.reactionSimilarity > 0.4) {
        return ReactionType::listening;
    } else {
        return ReactionType::thinking;
    }
}

DirectiveType GesturePerformerService::getBestDirectiveType(const LLMResponseData &llm_response) {
    // Implement your logic to determine if there's a directive command
    // This could parse the response text for command keywords

    // Example implementation - you should parse the actual response text
    return DirectiveType::go; // Default - no directive
}

bool GesturePerformerService::hasDirectiveCommand(const LLMResponseData &llm_response) {
    // Check if the response contains directive commands
    // Parse llm_response.responseText for commands like "go", "come", "stop", etc.

    // Example implementation - replace with actual text parsing
    std::string responseText = llm_response.sentence;
    std::transform(responseText.begin(), responseText.end(), responseText.begin(), ::tolower);

    return (responseText.find("go") != std::string::npos ||
            responseText.find("come") != std::string::npos ||
            responseText.find("stop") != std::string::npos ||
            responseText.find("look") != std::string::npos ||
            responseText.find("turn") != std::string::npos ||
            responseText.find("back") != std::string::npos);
}


void GesturePerformerService::subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data) {

    std::lock_guard<std::mutex> lock(_dataMutex);

    switch (type) {
        case MessageType::LLMResponse: {
            if (data) {
                LLMResponseData llmResponse = *static_cast<LLMResponseData*>(data.get());
                make_mimic(llmResponse);
            }
            break;
        }
        case MessageType::RecognizedGesture:{
            if (data) {
                std::string face_gesture = static_cast<RecognizedGestureData*>(data.get())->faceGesture;
                std::vector<int> face_landmark = static_cast<RecognizedGestureData*>(data.get())->faceLandmark;
                std::string hand_gesture = static_cast<RecognizedGestureData*>(data.get())->handGesture;
                std::vector<int> hand_landmark = static_cast<RecognizedGestureData*>(data.get())->handLandmark;

                //recognized_gesture(face_gesture, face_landmark, hand_gesture, hand_landmark);
            }
            break;
        }
        default:
            WARNING("{} subcribed_data_receive unknown message type!", get_service_name());
            break;
    }
}

