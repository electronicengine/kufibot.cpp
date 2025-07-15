#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <opencv2/opencv.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include <variant>

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
    std::string publisherName;
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

struct SensorData : public MessageData {
    Json sensorData;
};

struct ControlData : public MessageData {
    Json controlData;
};

struct LLMQueryData : public MessageData{
    std::string query;
};

struct LLMResponseData : public MessageData {
    std::string response;
};

struct RecognizedGestureData : public MessageData  {
    std::string faceGesture;
    std::vector<int> faceLandmark;
    std::string handGesture;
    std::vector<int> handLandmark;
};


class Subscriber {

public:
    virtual ~Subscriber() = default;

    virtual void subcribed_data_receive(MessageType type, MessageData* data) = 0;

};

#endif // SUBSCRIBER_H