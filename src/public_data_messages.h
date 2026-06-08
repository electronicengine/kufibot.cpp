//
// Created by ulak on 16.07.2025.
//

#ifndef PUBLIC_DATA_MESSAGES_H
#define PUBLIC_DATA_MESSAGES_H

#include "controllers/controller_data_structures.h"
#include <optional>
#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

#include "gesture_defs.h"

#define KUFI_HOME "/usr/local/"

// Define directional angle thresholds
constexpr int RIGHT_MIN = -45;
constexpr int RIGHT     = 0;
constexpr int RIGHT_MAX = 45;
constexpr int UP_MIN    = 45;
constexpr int UP        = 90;
constexpr int UP_MAX    = 135;
constexpr int LEFT_MIN  = 135;
constexpr int LEFT      = 180;
constexpr int LEFT_MAX  = -135; // wraps around
constexpr int DOWN_MIN  = -135;
constexpr int DOWN      = -90;
constexpr int DOWN_MAX  = -45;

using Json = nlohmann::json;


struct ConfigPaths {
    std::string ai_config;
    std::string gesture_config;
    std::string joint_angles;
    std::string motion_definitions;
    std::string rag_dataset;
    std::string reaction_set;
};

struct LlmSettings {
    std::string ngl;
    int nThreads;
    int n_ctx;
    double minP;
    double temp;
    int topK;
    double topP;
    std::string templateName;
};

struct SpeechProcessorConfig {
    std::string modelPath;
};


struct SpeechRecognizerConfig {
    std::string modelPath;
    std::string command;
    std::string voice;
    int timeOut;
    uint32_t silenceThreshold;
    uint32_t sampleRate;
    uint32_t framesPerBuffer;
    uint32_t maxSilenceDurationSec;
    uint32_t listenTimeoutMs;

};

struct LlmChatConfig {
    std::string modelPath;
    std::string systemMessage;
    LlmSettings llmSettings;
};

struct LlmEmbeddingConfig {
    std::string modelPath;
    int poolingType;
};

struct MediapipeConfig {
    std::string modelPath;
};

struct AiConfig {
    SpeechProcessorConfig speechProcessorConfig;
    SpeechRecognizerConfig speechRecognizerConfig;
    LlmChatConfig llmChatConfig;
    LlmEmbeddingConfig llmEmbeddingConfig;
    MediapipeConfig mediapipeConfig;
};


struct DatasetRow {
    std::string input;
    std::string output;
    std::vector<float> inputEmbedding;
};

struct RagDataset {
    std::vector<DatasetRow> rows;
};

enum class SourceService {
    none,
    expressionService,
    perceptionService,
    landmarkTrackerService,
    mappingService,
    tuiService,
    remoteControllerService,
    remoteConnectionService,
    interactiveChatService,
};

enum EventType {
    none = 0,
    critical_error,
    stop,
    timeout,
    control,
    listen_voice,
};


enum class MessageType {
    VideoFrame,
    SensorData,
    ControlData,
    DatabaseInsertData,
    LLMQuery,
    RecognizedSpeech,
    LLMResponse,
    EngageReaction,
    RecognizedGesture,
    GesturePerformanceCompleted,
    InteractiveChatStarted,
    SensorReadRequest,
    SpeakRequest,
    UpdateRAGDatabaseRequest,
    ClearRAGDatabaseRequest,
    ShowRAGDatabaseRequest,
    AIModeOnCall,
    AIModeOffCall,
    StopVideoStreamRequest,
    StartVideoStreamRequest
};

using Json = nlohmann::json;

struct MessageData {
    virtual ~MessageData() = default;
    std::optional<SourceService> source;
};

// Message types as simple structs
struct VideoFrameData : public MessageData {
    cv::Mat frame;
};


struct SensorData  : public MessageData {
    std::optional<CompassData> compassData;
    std::optional<DistanceData> distanceData;
    std::optional<PowerData> powerData;
    std::optional<std::map<ServoMotorJoint, uint8_t>> currentJointAngles;
    std::optional<DCMotorState> dcMotorState;

    [[nodiscard]] std::string to_json() const {

        if (!compassData.has_value() || !distanceData.has_value() || !powerData.has_value() ) {
            return "";
        }

        Json joint_angles = {
            {"right_arm", currentJointAngles->at(ServoMotorJoint::rightArm)},
            {"left_arm", currentJointAngles->at(ServoMotorJoint::leftArm)},
            {"neck_down", currentJointAngles->at(ServoMotorJoint::neck)},
            {"neck_up", currentJointAngles->at(ServoMotorJoint::headUpDown)},
            {"neck_right", currentJointAngles->at(ServoMotorJoint::headLeftRight)},
            {"eye_right", currentJointAngles->at(ServoMotorJoint::eyeRight)},
            {"eye_left", currentJointAngles->at(ServoMotorJoint::eyeLeft)}
        };

        Json compass = {
            {"angle", compassData->angle},
            {"magnet", {
                {"magnet_x", compassData->magnetX},
                {"magnet_y", compassData->magnetY}
            }}
        };

        Json distance = {
            {"Distance", distanceData->distance},
            {"Strength", distanceData->strength},
            {"Temperature", distanceData->temperature}
        };

        Json power = {
            {"BusVoltage", powerData->busVoltage},
            {"BusCurrent", powerData->current},
            {"Power", powerData->power},
            {"ShuntVoltage",powerData->shuntVoltage}
        };

        Json metadata = {
            {"joint_angles", joint_angles},
            {"compass", compass},
            {"distance", distance},
            {"power", power}
        };

        return metadata.dump();
    }
};

struct JoyStickData : public MessageData {
    int angle = 0;
    int strength = 0;
};

struct DatabaseInsertData : public MessageData {
    std::string input;
    std::string output;
};

struct ControlData : public MessageData {
    std::optional<std::pair<ServoMotorJoint, uint8_t>> jointAngle;
    std::optional<std::map<ServoMotorJoint, uint8_t>> jointAngles;
    std::optional<DCMotorState> dcMotorState;
    std::optional<JoyStickData> headJoystick;
    std::optional<JoyStickData> bodyJoystick;
    std::optional<int> leftArmAngle;
    std::optional<int> rightArmAngle;
    std::optional<bool> leftEye;
    std::optional<bool> rightEye;

    ControlData() = default;

    explicit ControlData(Json data) {
        if (!data.is_null() && data.is_object()) {
            if (!data.empty()) {
                std::string object_name = data.begin().key();
                const std::string& control_id = object_name;

                if (control_id == "body_joystick") {
                    bodyJoystick.emplace();
                    bodyJoystick->angle = data[object_name]["Angle"];
                    bodyJoystick->strength = data[object_name]["Strength"];
                } else if (control_id == "head_joystick") {
                    headJoystick.emplace();
                    headJoystick->angle = data[object_name]["Angle"];
                    headJoystick->strength = data[object_name]["Strength"];
                } else if (control_id == "right_arm" ) {
                    rightArmAngle = data[object_name]["Angle"];
                } else if (control_id == "left_arm") {
                    leftArmAngle = data[object_name]["Angle"];
                }else if (control_id == "left_eye" ) {
                    leftEye = data[object_name]["Angle"] == 180 ? true : false;
                } else if (control_id == "right_eye"){
                    rightEye = data[object_name]["Angle"] == 180 ? true : false;
                }
            }
        }
    }
};

struct LLMQueryData : public MessageData {
    std::string query;
};

struct RecognizedSpeechData : public MessageData {
    std::string text;
};

struct EngageReactionData : public MessageData {
    std::string reaction;
};

struct SpeakRequestData : public MessageData {
    std::string text;
};

struct LLMResponseData : public MessageData {
    std::string sentence;
    EmotionalGesture emotionalGesture;
    ReactionalGesture reactionalGesture;
    Directive directive;
    bool endMarker;

    float emotionSimilarity;
    float reactionSimilarity;
    float directiveSimilarity;


    LLMResponseData() = default;

    LLMResponseData(const std::string& jsonStr) {

        Json j = Json::parse(jsonStr);

        if (!j.is_object()) {
            return;
        }

        // Zorunlu alanlar (to_json tarafında yazılanlar)
        sentence = j.value("sentence", std::string{});

        emotionalGesture.symbol = j.value("emotional_gesture",std::string{});
        reactionalGesture.symbol = j.value("reactional_gesture", std::string{});
        directive.symbol = j.value("directive", std::string{});

        endMarker = j.value("end_marker", false);

        // Similarity değerleri yoksa 0.0f default
        emotionSimilarity = j.value("emotion_similarity", 0.0f);
        reactionSimilarity = j.value("reaction_similarity", 0.0f);
        directiveSimilarity = j.value("directive_similarity", 0.0f);

    }


    [[nodiscard]] std::string to_json() const {
        Json j;
        j["sentence"] = sentence;
        j["emotional_gesture"] = emotionalGesture.symbol;
        j["reactional_gesture"] = reactionalGesture.symbol;
        j["directive"] = directive.symbol;
        j["end_marker"] = endMarker;
        j["emotion_similarity"] = emotionSimilarity;
        j["reaction_similarity"] = reactionSimilarity;
        j["directive_similarity"] = directiveSimilarity;
        return j.dump();
    }
};

struct BoundingBox {
    int xmin;
    int ymin;
    int xmax;
    int ymax;
    bool valid;

    BoundingBox() : xmin(0), ymin(0), xmax(0), ymax(0), valid(false) {}
    BoundingBox(int x1, int y1, int x2, int y2)
        : xmin(x1), ymin(y1), xmax(x2), ymax(y2), valid(true) {}
};


struct Landmark {
    int id;
    int cx;
    int cy;
};

struct FaceInfo {
    double left_ear;
    double right_ear;
    double avg_ear;
    double mar;
    double eyebrow_height;

    FaceInfo() : left_ear(0), right_ear(0), avg_ear(0), mar(0), eyebrow_height(0) {}
};


struct RecognizedGestureData : public MessageData {
    std::string faceEmotion;
    FaceInfo faceInfo;
    std::vector<Landmark> faceLandmarks;
    std::string handGesture;
    BoundingBox handBbox;
    std::vector<Landmark> handLandmarks;
};



#endif //PUBLIC_DATA_MESSAGES_H
