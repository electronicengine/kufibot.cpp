//
// Created by ulak on 16.07.2025.
//

#ifndef PUBLIC_DATA_MESSAGES_H
#define PUBLIC_DATA_MESSAGES_H

#include "controllers/controller_data_structures.h"
#include <optional>
#include <opencv2/opencv.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>

#include "gesture_defs.h"

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

typedef websocketpp::server<websocketpp::config::asio> Server;
using Json = nlohmann::json;


enum class MessageType {
    VideoFrame,
    WebSocketReceive,
    WebSocketTransfer,
    SensorData,
    ControlData,
    LLMQuery,
    LLMResponse,
    RecognizedGesture
};

typedef websocketpp::server<websocketpp::config::asio> Server;
using Json = nlohmann::json;

struct MessageData {
    std::optional<std::string> publisherName;
};

// Message types as simple structs
struct VideoFrameData : public MessageData{
    cv::Mat frame;
};

struct WebSocketReceiveData : public MessageData{
    websocketpp::connection_hdl hdl;
    std::string msg;
};

struct WebSocketTransferData : public MessageData {
    websocketpp::connection_hdl hdl;
    std::string msg;
    uint8_t type;
};

struct SensorData  : public MessageData {
    std::optional<CompassData> compassData;
    std::optional<DistanceData> distanceData;
    std::optional<PowerData> powerData;
    std::optional<std::map<ServoMotorJoint, uint8_t>> currentJointAngles;
    std::optional<DCMotorState> dcMotorState;

    Json to_json() const {
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

        return metadata;
    }


};

struct JoyStickData : public MessageData {
    int angle;
    int strength;
};

struct ControlData : public MessageData {
    std::optional<std::map<ServoMotorJoint, uint8_t>> jointAngles;
    std::optional<DCMotorState> dcMotorState;
    std::optional<JoyStickData> headJoystick;
    std::optional<JoyStickData> bodyJoystick;
    std::optional<int> leftArmAngle;
    std::optional<int> rightArmAngle;
    std::optional<bool> leftEye;
    std::optional<bool> rightEye;

    ControlData() = default;

    ControlData(Json data) {
        if (!data.is_null() && data.is_object()) {
            if (!data.empty()) {
                std::string object_name = data.begin().key();
                std::string control_id = object_name;

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

struct LLMQueryData : public MessageData{
    std::string query;
};

struct LLMResponseData : public MessageData {
    std::string sentence;
    EmotionType emotion;
    ReactionType reaction;
    float emotionSimilarity;
    float reactionSimilarity;
};

struct RecognizedGestureData : public MessageData  {
    std::string faceGesture;
    std::vector<int> faceLandmark;
    std::string handGesture;
    std::vector<int> handLandmark;
};



#endif //PUBLIC_DATA_MESSAGES_H
