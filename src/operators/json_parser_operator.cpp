#include "json_parser_operator.h"
#include <fstream>
#include <stdexcept>
#include "../logger.h"

using json = nlohmann::json;

JsonParserOperator* JsonParserOperator::_instance = nullptr;

JsonParserOperator* JsonParserOperator::get_instance() {
    if (_instance == nullptr) {
        _instance = new JsonParserOperator();
    }
    return _instance;
}

// ======================= STRING ↔ ENUM =======================

JsonParserOperator::JsonParserOperator() {
}

EmotionType JsonParserOperator::emotionTypeFromString(const std::string& str) {
    if (str == "happy") return EmotionType::happy;
    if (str == "angry") return EmotionType::angry;
    if (str == "funny") return EmotionType::funny;
    if (str == "serious") return EmotionType::serious;
    if (str == "curious") return EmotionType::curious;
    if (str == "worried") return EmotionType::worried;
    if (str == "surprised") return EmotionType::surprised;
    if (str == "confident") return EmotionType::confident;
    ERROR("Unknown EmotionType: {}", str);
    return EmotionType::angry; // default value
}

ReactionType JsonParserOperator::reactionTypeFromString(const std::string& str) {
    if (str == "greeting") return ReactionType::greeting;
    if (str == "listening") return ReactionType::listening;
    if (str == "talking") return ReactionType::talking;
    if (str == "accepting") return ReactionType::accepting;
    if (str == "rejecting") return ReactionType::rejecting;
    if (str == "thinking") return ReactionType::thinking;
    if (str == "agreeing") return ReactionType::agreeing;
    ERROR("Unknown ReactionType: ", str);
    return ReactionType::greeting; // default value
}

DirectiveType JsonParserOperator::directiveTypeFromString(const std::string& str) {
    if (str == "go") return DirectiveType::go;
    if (str == "come") return DirectiveType::come;
    if (str == "stop") return DirectiveType::stop;
    if (str == "look") return DirectiveType::look;
    if (str == "turnAround") return DirectiveType::turnAround;
    if (str == "standBack") return DirectiveType::standBack;
    ERROR("Unknown DirectiveType: {}", str);
    return DirectiveType::go; // default value
}

ServoMotorJoint JsonParserOperator::servoJointFromString(const std::string& str) {
    if (str == "leftArm") return ServoMotorJoint::leftArm;
    if (str == "rightArm") return ServoMotorJoint::rightArm;
    if (str == "neck") return ServoMotorJoint::neck;
    if (str == "headUpDown") return ServoMotorJoint::headUpDown;
    if (str == "headLeftRight") return ServoMotorJoint::headLeftRight;
    if (str == "eyeLeft") return ServoMotorJoint::eyeLeft;
    if (str == "eyeRight") return ServoMotorJoint::eyeRight;

    ERROR("Unknown ServoMotorJoint: {}", str);
    return ServoMotorJoint::eyeLeft; // default value
}

GestureJointState JsonParserOperator::gestureStateFromString(const std::string& str) {
    if (str == "fullup") return GestureJointState::fullup;
    if (str == "halfup") return GestureJointState::halfup;
    if (str == "middle") return GestureJointState::middle;
    if (str == "halfdown") return GestureJointState::halfdown;
    if (str == "fulldown") return GestureJointState::fulldown;
    ERROR("Unknown GestureJointState: {}", str);
    return GestureJointState::fulldown; // default value
}

std::string JsonParserOperator::to_string(ServoMotorJoint joint) {
    switch (joint) {
        case ServoMotorJoint::leftArm: return "leftArm";
        case ServoMotorJoint::rightArm: return "rightArm";
        case ServoMotorJoint::neck: return "neck";
        case ServoMotorJoint::headUpDown: return "headUpDown";
        case ServoMotorJoint::headLeftRight: return "headLeftRight";
        case ServoMotorJoint::eyeLeft: return "eyeLeft";
        case ServoMotorJoint::eyeRight: return "eyeRight";
        default: return "unknown";
    }
}

std::string JsonParserOperator::to_string(GestureJointState state) {
    switch (state) {
        case GestureJointState::fullup: return "fullup";
        case GestureJointState::halfup: return "halfup";
        case GestureJointState::middle: return "middle";
        case GestureJointState::halfdown: return "halfdown";
        case GestureJointState::fulldown: return "fulldown";
    }
    return "unknown";
}

std::string JsonParserOperator::to_string(EmotionType emotion) {
    switch (emotion) {
        case EmotionType::happy: return "happy";
        case EmotionType::angry: return "angry";
        case EmotionType::funny: return "funny";
        case EmotionType::serious: return "serious";
        case EmotionType::curious: return "curious";
        case EmotionType::worried: return "worried";
        case EmotionType::surprised: return "surprised";
        case EmotionType::confident: return "confident";
        default: return "unknown";
    }
}

std::string JsonParserOperator::to_string(ReactionType reaction) {
    switch (reaction) {
        case ReactionType::greeting: return "greeting";
        case ReactionType::listening: return "listening";
        case ReactionType::talking: return "talking";
        case ReactionType::accepting: return "accepting";
        case ReactionType::rejecting: return "rejecting";
        case ReactionType::thinking: return "thinking";
        case ReactionType::agreeing: return "agreeing";
        default: return "unknown";
    }
}

std::string JsonParserOperator::to_string(DirectiveType directive) {
    switch (directive) {
        case DirectiveType::go: return "go";
        case DirectiveType::come: return "come";
        case DirectiveType::stop: return "stop";
        case DirectiveType::look: return "look";
        case DirectiveType::turnAround: return "turnAround";
        case DirectiveType::standBack: return "standBack";
        default: return "unknown";
    }
}

// ======================= MOTION JSON PARSERS =======================

MotionSequenceItem JsonParserOperator::parseMotionSequenceItem(const json& sequenceJson) {
    MotionSequenceItem item;
    item.time = sequenceJson["time"];

    for (const auto& [jointKey, stateKey] : sequenceJson["joints"].items()) {
        ServoMotorJoint joint = servoJointFromString(jointKey);
        GestureJointState state = gestureStateFromString(stateKey);
        item.joints[joint] = state;
    }

    return item;
}

std::map<ServoMotorJoint, GestureJointState> JsonParserOperator::parseJointPositions(const json& jointsJson) {
    std::map<ServoMotorJoint, GestureJointState> positions;

    for (const auto& [jointKey, stateKey] : jointsJson.items()) {
        ServoMotorJoint joint = servoJointFromString(jointKey);
        GestureJointState state = gestureStateFromString(stateKey);
        positions[joint] = state;
    }

    return positions;
}

EmotionalMotion JsonParserOperator::parseEmotionalMotion(const json& motionJson) {
    EmotionalMotion motion;
    motion.name = motionJson["name"];
    motion.description = motionJson["description"];
    motion.duration = motionJson["duration"];
    motion.joints = parseJointPositions(motionJson["joints"]);

    // Parse sequence if it exists
    if (motionJson.contains("sequence")) {
        for (const auto& seqItem : motionJson["sequence"]) {
            motion.sequence.push_back(parseMotionSequenceItem(seqItem));
        }
    }

    return motion;
}

ReactionalMotion JsonParserOperator::parseReactionalMotion(const json& motionJson) {
    ReactionalMotion motion;
    motion.name = motionJson["name"];
    motion.description = motionJson["description"];
    motion.duration = motionJson["duration"];
    motion.joints = parseJointPositions(motionJson["joints"]);

    // Parse sequence if it exists
    if (motionJson.contains("sequence")) {
        for (const auto& seqItem : motionJson["sequence"]) {
            motion.sequence.push_back(parseMotionSequenceItem(seqItem));
        }
    }

    return motion;
}

DirectiveMotion JsonParserOperator::parseDirectiveMotion(const json& motionJson) {
    DirectiveMotion motion;
    motion.name = motionJson["name"];
    motion.description = motionJson["description"];
    motion.duration = motionJson["duration"];
    motion.joints = parseJointPositions(motionJson["joints"]);

    // Parse sequence if it exists
    if (motionJson.contains("sequence")) {
        for (const auto& seqItem : motionJson["sequence"]) {
            motion.sequence.push_back(parseMotionSequenceItem(seqItem));
        }
    }

    return motion;
}

void JsonParserOperator::loadMotionsFromFile(
    const std::string& filename,
    std::map<EmotionType, EmotionalMotion>& emotionalMotions,
    std::map<ReactionType, ReactionalMotion>& reactionalMotions,
    std::map<DirectiveType, DirectiveMotion>& directiveMotions,
    std::map<ServoMotorJoint, GestureJointState>& idlePosition) {

    try {
        std::ifstream inFile(filename);
        if (!inFile.is_open()) {
            ERROR("Failed to open motion file: {}", filename);
            return;
        }

        json j;
        inFile >> j;

        // Load emotional gestures
        if (j.contains("motions") && j["motions"].contains("emotional_gestures")) {
            for (const auto& [emotionKey, motionData] : j["motions"]["emotional_gestures"].items()) {
                EmotionType emotionType = emotionTypeFromString(emotionKey);
                emotionalMotions[emotionType] = parseEmotionalMotion(motionData);
                INFO("Loaded emotional motion: {}", emotionKey);
            }
        }

        // Load reactional gestures
        if (j.contains("motions") && j["motions"].contains("reactional_gestures")) {
            for (const auto& [reactionKey, motionData] : j["motions"]["reactional_gestures"].items()) {
                ReactionType reactionType = reactionTypeFromString(reactionKey);
                reactionalMotions[reactionType] = parseReactionalMotion(motionData);
                INFO("Loaded reactional motion: {}", reactionKey);
            }
        }

        // Load directive motions
        if (j.contains("motions") && j["motions"].contains("directives")) {
            for (const auto& [directiveKey, motionData] : j["motions"]["directives"].items()) {
                DirectiveType directiveType = directiveTypeFromString(directiveKey);
                directiveMotions[directiveType] = parseDirectiveMotion(motionData);
                INFO("Loaded directive motion: {}", directiveKey);
            }
        }

        // Load default idle position
        if (j.contains("default_idle") && j["default_idle"].contains("joints")) {
            idlePosition = parseJointPositions(j["default_idle"]["joints"]);
            INFO("Loaded idle position");
        }

        INFO("Successfully loaded motion definitions from: {}", filename);

    } catch (const std::exception& e) {
        ERROR("Error loading motion file {}: {}", filename, e.what());
    }
}

void JsonParserOperator::writeMotionsToFile(
    const std::string& filename,
    const std::map<EmotionType, EmotionalMotion>& emotionalMotions,
    const std::map<ReactionType, ReactionalMotion>& reactionalMotions,
    const std::map<DirectiveType, DirectiveMotion>& directiveMotions,
    const std::map<ServoMotorJoint, GestureJointState>& idlePosition) {

    try {
        json j;

        // Write emotional motions
        for (const auto& [emotionType, motion] : emotionalMotions) {
            std::string emotionKey = to_string(emotionType);
            json motionJson;
            motionJson["name"] = motion.name;
            motionJson["description"] = motion.description;
            motionJson["duration"] = motion.duration;

            // Write joints
            for (const auto& [joint, state] : motion.joints) {
                motionJson["joints"][to_string(joint)] = to_string(state);
            }

            // Write sequence if not empty
            if (!motion.sequence.empty()) {
                for (const auto& seqItem : motion.sequence) {
                    json seqJson;
                    seqJson["time"] = seqItem.time;
                    for (const auto& [joint, state] : seqItem.joints) {
                        seqJson["joints"][to_string(joint)] = to_string(state);
                    }
                    motionJson["sequence"].push_back(seqJson);
                }
            }

            j["motions"]["emotional_gestures"][emotionKey] = motionJson;
        }

        // Write reactional motions
        for (const auto& [reactionType, motion] : reactionalMotions) {
            std::string reactionKey = to_string(reactionType);
            json motionJson;
            motionJson["name"] = motion.name;
            motionJson["description"] = motion.description;
            motionJson["duration"] = motion.duration;

            // Write joints
            for (const auto& [joint, state] : motion.joints) {
                motionJson["joints"][to_string(joint)] = to_string(state);
            }

            // Write sequence if not empty
            if (!motion.sequence.empty()) {
                for (const auto& seqItem : motion.sequence) {
                    json seqJson;
                    seqJson["time"] = seqItem.time;
                    for (const auto& [joint, state] : seqItem.joints) {
                        seqJson["joints"][to_string(joint)] = to_string(state);
                    }
                    motionJson["sequence"].push_back(seqJson);
                }
            }

            j["motions"]["reactional_gestures"][reactionKey] = motionJson;
        }

        // Write directive motions
        for (const auto& [directiveType, motion] : directiveMotions) {
            std::string directiveKey = to_string(directiveType);
            json motionJson;
            motionJson["name"] = motion.name;
            motionJson["description"] = motion.description;
            motionJson["duration"] = motion.duration;

            // Write joints
            for (const auto& [joint, state] : motion.joints) {
                motionJson["joints"][to_string(joint)] = to_string(state);
            }

            // Write sequence if not empty
            if (!motion.sequence.empty()) {
                for (const auto& seqItem : motion.sequence) {
                    json seqJson;
                    seqJson["time"] = seqItem.time;
                    for (const auto& [joint, state] : seqItem.joints) {
                        seqJson["joints"][to_string(joint)] = to_string(state);
                    }
                    motionJson["sequence"].push_back(seqJson);
                }
            }

            j["motions"]["directives"][directiveKey] = motionJson;
        }

        // Write idle position
        for (const auto& [joint, state] : idlePosition) {
            j["default_idle"]["joints"][to_string(joint)] = to_string(state);
        }
        j["default_idle"]["name"] = "idle";
        j["default_idle"]["description"] = "Neutral resting position";

        // Write transition settings
        j["transition_settings"]["default_transition_time"] = 500;
        j["transition_settings"]["smooth_interpolation"] = true;
        j["transition_settings"]["ease_type"] = "ease-in-out";

        std::ofstream file(filename);
        file << j.dump(4);

        INFO("Successfully wrote motion definitions to: {}", filename);

    } catch (const std::exception& e) {
        ERROR("Error writing motion file {}: {}", filename, e.what());
    }
}

// ======================= ORIGINAL JSON LOADERS =======================

void JsonParserOperator::loadGesturesFromFile(
    const std::string& filename,
    std::list<EmotionalGesture>& emotionalList,
    std::list<ReactionalGesture>& reactionalList,
    std::list<Directive>& directiveList) {

    std::ifstream inFile(filename);
    json j;
    inFile >> j;

    for (const auto& item : j["emotional_gestures"]) {
        emotionalList.push_back({
            emotionTypeFromString(item["type"]),
            item["tag"],
            item["description"],
            {}, // motorPos (varsayılan boş değer)
            {}  // embedding (varsayılan boş değer)
        });
    }

    for (const auto& item : j["reactional_gestures"]) {
        reactionalList.push_back({
            reactionTypeFromString(item["type"]),
            reactionTypeFromString(item["reply"]),
            item["tag"],
            item["description"],
            {} // embedding
        });
    }

    for (const auto& item : j["directives"]) {
        directiveList.push_back({
            directiveTypeFromString(item["type"]),
            item["tag"],
            item["description"],
            {} // embedding
        });
    }
}

void JsonParserOperator::loadGestureJointAnglesFromJson(
    const std::string& filename,
    std::map<ServoMotorJoint, std::map<GestureJointState, GestureJointAngle>>& data) {

    std::ifstream file(filename);
    json j;
    file >> j;

    for (auto& [jointKey, stateMap] : j.items()) {
        ServoMotorJoint joint = servoJointFromString(jointKey);
        for (auto& [stateKey, value] : stateMap.items()) {
            GestureJointState state = gestureStateFromString(stateKey);
            GestureJointAngle angle;
            angle.angle = value["angle"];
            angle.symbol = value["name"];
            data[joint][state] = angle;
        }
    }
}

void JsonParserOperator::writeGestureJointAnglesToJson(
    const std::map<ServoMotorJoint, std::map<GestureJointState, GestureJointAngle>>& data,
    const std::string& filename) {

    json j;

    for (const auto& [joint, states] : data) {
        std::string jointStr = to_string(joint);
        for (const auto& [state, angleInfo] : states) {
            std::string stateStr = to_string(state);
            j[jointStr][stateStr]["angle"] = angleInfo.angle;
            j[jointStr][stateStr]["name"] = angleInfo.symbol;
        }
    }

    std::ofstream file(filename);
    file << j.dump(4);
}