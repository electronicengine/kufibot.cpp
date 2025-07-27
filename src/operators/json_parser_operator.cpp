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

// ======================= JSON LOADERS =======================

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
