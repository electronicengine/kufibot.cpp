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

#include "json_parser_operator.h"
#include <fstream>
#include <stdexcept>
#include "../logger.h"

using json = nlohmann::json;

JsonParserOperator* JsonParserOperator::_instance = nullptr;

JsonParserOperator *JsonParserOperator::get_instance() {
    if (_instance == nullptr) {
        _instance = new JsonParserOperator();
    }
    return _instance;
}


// ======================= STRING ↔ ENUM =======================

JsonParserOperator::JsonParserOperator() {

    loadConfigPaths(CONFIG_PATHS_FILE, _configPaths.emplace());
    if (_configPaths->ai_config.empty()) {
        _configPaths.reset();
        ERROR("_configPaths.ai_config is empty. Please check the config paths file: {}", CONFIG_PATHS_FILE);
    }else {

        loadRagDataset(_configPaths->rag_dataset, _ragDataset.emplace());
        loadFaceReactions(_configPaths->reaction_set, _faceReactionList.emplace());

        loadMotionsFromFile(_configPaths->motion_definitions, _emotionalMotions.emplace(), _reactionalMotions.emplace(), _directiveMotions.emplace(), _idlePosition.emplace());

        if (_emotionalMotions->empty()) {
            ERROR("No emotional motions found in the motion definitions file: {}", _configPaths->motion_definitions);
            _emotionalMotions.reset();
            _reactionalMotions.reset();
            _directiveMotions.reset();
            _idlePosition.reset();
        }

        loadGesturesFromFile(_configPaths->gesture_config, _emotionalList.emplace(), _reactionalList.emplace(), _directiveList.emplace());
        if (_emotionalMotions->empty()) {
            ERROR("No emotional gestures found in the gesture definitions file: {}", _configPaths->gesture_config);
            _emotionalList.reset();
            _reactionalList.reset();
            _directiveList.reset();
        }

        loadJointAnglesFromJson(_configPaths->joint_angles, _jointAngles.emplace());
        if (_jointAngles->empty()) {
            ERROR("No Joint found in the joint angle file: {}", _configPaths->joint_angles);
            _jointAngles.reset();
        }

        loadAiConfig(_configPaths->ai_config, _aiConfig.emplace());
        if (_aiConfig->speechProcessorConfig.modelPath.empty()) {
            _aiConfig.reset();
            ERROR("No speech processor model path found in the AI config file: {}", _configPaths->ai_config);
        }
    }

}


void JsonParserOperator::loadConfigPaths(const std::string& filename, ConfigPaths& paths) {
    try {
        std::ifstream inFile(filename);
        if (!inFile.is_open()) {
            ERROR("Failed to open config file: {}", filename);
        }

        json j;
        inFile >> j;

        paths.ai_config = j.value("ai_config", "");
        paths.gesture_config = j.value("gesture_config", "");
        paths.joint_angles = j.value("joint_angles", "");
        paths.motion_definitions = j.value("motion_definitions", "");
        paths.rag_dataset = j.value("rag_dataset", "");
        paths.reaction_set = j.value("reaction_set", "");

    } catch (const std::exception& e) {
        ERROR("Error parsing config paths file {} : {} " , filename ,e.what());
    }
}

void JsonParserOperator::loadAiConfig(const std::string &filename, AiConfig &config) {

    try {
        std::ifstream inFile(filename);
        if (!inFile.is_open()) {
            ERROR("Failed to open AI config file: {}", filename);
        }

        json j;
        inFile >> j;

        // speechProcessor
        if (j.contains("speechPrecessor")) { // note: key in JSON is 'speechPrecessor' (typo kept intentionally if file
                                             // uses that)
            const auto &sp = j["speechPrecessor"];
            config.speechProcessorConfig.modelPath = sp.value("modelPath", "");
        }

        // speechRecognizer
        if (j.contains("speechRecognizer")) {
            const auto &sr = j["speechRecognizer"];
            config.speechRecognizerConfig.modelPath = sr.value("modelPath", "");
            config.speechRecognizerConfig.command = sr.value("command", "");
            config.speechRecognizerConfig.voice = sr.value("voice", "");
            config.speechRecognizerConfig.timeOut = sr.value("timeOut", 0);
            config.speechRecognizerConfig.silenceThreshold = sr.value("silenceThreshold", 0);
            config.speechRecognizerConfig.sampleRate = sr.value("sampleRate", 0);
            config.speechRecognizerConfig.framesPerBuffer = sr.value("framesPerBuffer", 0);
            config.speechRecognizerConfig.maxSilenceDurationSec = sr.value("maxSilenceDurationSec", 0);
            config.speechRecognizerConfig.listenTimeoutMs = sr.value("listenTimeoutMs", 0);

        }

        // llmChat
        if (j.contains("llmChat")) {
            const auto &lc = j["llmChat"];
            config.llmChatConfig.modelPath = lc.value("modelPath", "");
            config.llmChatConfig.systemMessage = lc.value("systemMessage", "");

            if (lc.contains("settings")) {
                const auto &s = lc["settings"];
                config.llmChatConfig.llmSettings.ngl = s.value("ngl", "");
                config.llmChatConfig.llmSettings.nThreads = s.value("nThreads", 0);
                config.llmChatConfig.llmSettings.n_ctx = s.value("n_ctx", 0);
                config.llmChatConfig.llmSettings.minP = s.value("minP", 0.0);
                config.llmChatConfig.llmSettings.temp = s.value("temp", 0.0);
                config.llmChatConfig.llmSettings.topK = s.value("topK", 0);
                config.llmChatConfig.llmSettings.topP = s.value("topP", 0.0);
                config.llmChatConfig.llmSettings.templateName = s.value("template", "");
            }
        }

        // llmEmbedding
        if (j.contains("llmEmbedding")) {
            const auto &le = j["llmEmbedding"];
            config.llmEmbeddingConfig.modelPath = le.value("modelPath", "");
            config.llmEmbeddingConfig.poolingType = le.value("poolingType", 0);
        }

        // mediapipe
        if (j.contains("mediapipe")) {
            const auto &mp = j["mediapipe"];
            config.mediapipeConfig.modelPath = mp.value("modelPath", "");
        }

        INFO("AI configuration loaded successfully from: {}", filename);
    } catch (const std::exception &e) {
        ERROR("Error parsing AI config file {}: {}", filename, e.what());
    }
}


void JsonParserOperator::loadRagDataset(const std::string &filename, RagDataset &dataset) {

    try {
        std::ifstream inFile(filename);
        if (!inFile.is_open()) {
            ERROR("Failed to open RAG dataset file: {}", filename);
            return;
        }

        std::string line;
        while (std::getline(inFile, line)) {
            try {
                json lineJson = json::parse(line);
                std::string input = lineJson.value("input", "");
                std::string output = lineJson.value("output", "");
                dataset.rows.push_back(DatasetRow{input, output, {}});

            } catch (const json::parse_error &e) {
                ERROR("Error parsing JSON line in RAG dataset: {}", e.what());
                continue;
            }
        }
        INFO("Successfully loaded {} items from RAG dataset", dataset.rows.size());
    } catch (const std::exception &e) {
        ERROR("Error reading RAG dataset file {}: {}", filename, e.what());
    }
}

void JsonParserOperator::loadFaceReactions(const std::string &filename, std::list<FaceReaction> &faceReactionList) {
    try {
        std::ifstream inFile(filename);
        if (!inFile.is_open()) {
            ERROR("Failed to open RAG dataset file: {}", filename);
            return;
        }
        std::string line;
        while (std::getline(inFile, line)) {
            try {
                json lineJson = json::parse(line);
                std::string gesture = lineJson.value("gesture", "");
                std::string reaction = lineJson.value("reaction", "");
                faceReactionList.push_back(FaceReaction{gesture, reaction});

            } catch (const json::parse_error &e) {
                ERROR("Error parsing JSON line in reaction set: {}", e.what());
                continue;
            }
        }
        INFO("Successfully loaded {} items from  reaction set", faceReactionList.size());
    } catch (const std::exception &e) {
        ERROR("Error reading reaction set file {}: {}", filename, e.what());
    }
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
    if (str == "followFinger") return DirectiveType::followFinger;
    if (str == "stopFollow") return DirectiveType::stopFollow;

    ERROR("Unknown DirectiveType: {}", str);
    return DirectiveType::followFinger; // default value
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
        case DirectiveType::followFinger: return "followFinger";
        case DirectiveType::stopFollow: return "stopFollow";

        default: return "unknown";
    }
}


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

DirectiveMotion JsonParserOperator::parseDirectiveMotion(const json &motionJson) {
    DirectiveMotion motion;
    motion.name = motionJson["name"];
    motion.description = motionJson["description"];
    motion.duration = motionJson["duration"];
    motion.joints = parseJointPositions(motionJson["joints"]);

    // Parse sequence if it exists
    if (motionJson.contains("sequence")) {
        for (const auto &seqItem: motionJson["sequence"]) {
            motion.sequence.push_back(parseMotionSequenceItem(seqItem));
        }
    }

    return motion;
}

std::map<ServoMotorJoint, std::map<GestureJointState, GestureJointAngle>>
JsonParserOperator::getJointLimits(const std::string& filename) {

    std::ifstream inFile(filename);

    if (!inFile.is_open()) {
        ERROR("Failed to open motion file: {}", filename);
        return std::map<ServoMotorJoint, std::map<GestureJointState, GestureJointAngle>>();
    }

    json j = json::parse(inFile);

    std::map<ServoMotorJoint, std::map<GestureJointState, GestureJointAngle>> result;

    for (auto &[jointName, states] : j.items()) {
        ServoMotorJoint joint = servoJointFromString(jointName);

        for (auto &[stateName, data] : states.items()) {
            GestureJointState state = gestureStateFromString(stateName);
            GestureJointAngle angleData{
                data.value("angle", 0),
                data.value("name", "")
            };
            result[joint][state] = angleData;
        }
    }

    return result;

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
            }
        }

        // Load reactional gestures
        if (j.contains("motions") && j["motions"].contains("reactional_gestures")) {
            for (const auto& [reactionKey, motionData] : j["motions"]["reactional_gestures"].items()) {
                ReactionType reactionType = reactionTypeFromString(reactionKey);
                reactionalMotions[reactionType] = parseReactionalMotion(motionData);
            }
        }

        // Load directive motions
        if (j.contains("motions") && j["motions"].contains("directives")) {
            for (const auto& [directiveKey, motionData] : j["motions"]["directives"].items()) {
                DirectiveType directiveType = directiveTypeFromString(directiveKey);
                directiveMotions[directiveType] = parseDirectiveMotion(motionData);
            }
        }

        // Load default idle position
        if (j.contains("default_idle") && j["default_idle"].contains("joints")) {
            idlePosition = parseJointPositions(j["default_idle"]["joints"]);
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

void JsonParserOperator::loadJointAnglesFromJson(
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
