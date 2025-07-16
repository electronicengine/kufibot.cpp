#include "remote_connection_service.h"
#include "../logger.h"

RemoteConnectionService* RemoteConnectionService::_instance = nullptr;


RemoteConnectionService *RemoteConnectionService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new RemoteConnectionService();
    }
    return _instance;
}

RemoteConnectionService::RemoteConnectionService(int port) : Service("RemoteConnectionService")
{
    _port = port;
}

void RemoteConnectionService::service_function() {

    _webSocketService = WebSocketService::get_instance();
    _videoStreamService = VideoStreamService::get_instance();
    _robotControllerService = RobotControllerService::get_instance();

    subscribe_to_service(_webSocketService);
}

RemoteConnectionService::~RemoteConnectionService()
{
    stop(); // Ensure everything is stopped in the destructor
}


void RemoteConnectionService::video_frame(const cv::Mat& frame) {

    std::vector<uchar> buffer;
    cv::imencode(".jpg", frame, buffer);

    try {
        std::unique_ptr<MessageData> data= std::make_unique<WebSocketTransferData>();
        static_cast<WebSocketTransferData*>(data.get())->hdl = _hdl;
        static_cast<WebSocketTransferData*>(data.get())->msg = std::string(buffer.begin(), buffer.end());
        static_cast<WebSocketTransferData*>(data.get())->type = 2;
        publish(MessageType::WebSocketTransfer, data);

        static_cast<WebSocketTransferData*>(data.get())->hdl = _hdl;
        static_cast<WebSocketTransferData*>(data.get())->msg = _sensorData.to_json().dump();
        static_cast<WebSocketTransferData*>(data.get())->type = 1;
        publish(MessageType::WebSocketTransfer, data);

    } catch (const std::exception& e) {
        Logger::error("WebSocket Error: {} " + std::string(e.what()));
    }

}

void RemoteConnectionService::web_socket_receive_message(websocketpp::connection_hdl hdl,  const std::string& msg)
{
    if(msg == "on_open"){
        Logger::trace("new web socket connection extablished");
        _hdl = hdl;
        subscribe_to_service(_videoStreamService);
        subscribe_to_service(_robotControllerService);
    }
    else if(msg == "on_close"){
        unsubscribe_from_service(_videoStreamService);
        unsubscribe_from_service(_robotControllerService);
    }
    else{
        Json message = Json::parse(msg);
        if (message.contains("talkie")) {
            return;
        }else{
            std::unique_ptr<MessageData> data = std::make_unique<ControlData>();
            *static_cast<ControlData*>(data.get()) = ControlData(message);
            publish(MessageType::ControlData, data);
        }
    }
}

void RemoteConnectionService::subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data) {
    std::lock_guard<std::mutex> lock(_dataMutex);

    switch (type) {
        case MessageType::WebSocketReceive: {
            if (data) {
                std::string msg = static_cast<WebSocketReceiveData*>(data.get())->msg;
                websocketpp::connection_hdl hdl = static_cast<WebSocketReceiveData*>(data.get())->hdl;
                web_socket_receive_message(hdl, msg);
            }
            break;
        }

        case MessageType::VideoFrame: {
            if (data) {
                cv::Mat frame = static_cast<VideoFrameData*>(data.get())->frame;
                video_frame(frame);
            }

            break;
        }

        case MessageType::SensorData: {
            if (data) {
                _sensorData = *static_cast<SensorData*>(data.get());
            }

            break;
        }
        default:
            Logger::warn("{} subcribed_data_receive unknown message type!", get_service_name());
            break;


    }

}
