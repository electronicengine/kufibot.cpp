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
    _webSocket = WebSocketService::get_instance();
    _videoStream = VideoStreamService::get_instance();
    _robotControllerService = RobotControllerService::get_instance();
}

RemoteConnectionService::~RemoteConnectionService()
{
    stop(); // Ensure everything is stopped in the destructor
}

void RemoteConnectionService::start() {

    _webSocket->subscribe(this);
    _sensor_values = _robotControllerService->get_sensor_values();
    Logger::info("RemoteConnectionService is starting...");
}


void RemoteConnectionService::stop() {
     Logger::info("RemoteConnectionService is stopping...");

     Logger::info("RemoteConnectionService::_webSocket::un_subscribe");
    _webSocket->un_subscribe(this); 
     Logger::info("RemoteConnectionService::_videoStream::un_subscribe");
    _videoStream->un_subscribe(this); 
     Logger::info("RemoteConnectionService::_robotControllerService::un_subscribe");
    _robotControllerService->un_subscribe(this);
}


void RemoteConnectionService::update_video_frame(const cv::Mat& frame) {

    std::vector<uchar> buffer;
    cv::imencode(".jpg", frame, buffer);

    try {
        _webSocket->send_data(_hdl, buffer);

        std::string metadata_string = _sensor_values.dump();
        _webSocket->send_message(_hdl, metadata_string);

    } catch (const std::exception& e) {
        Logger::error("WebSocket Error: {} " + std::string(e.what()));
    }

}

void RemoteConnectionService::update_web_socket_message(websocketpp::connection_hdl hdl,  const std::string& msg)
{
    if(msg == "on_open"){
        Logger::trace("new web socket connection extablished");
        _hdl = hdl;
        Logger::trace("RemoteConnectionService::_videoStream::subscribe");
        _videoStream->subscribe(this);
        Logger::trace("RemoteConnectionService::_robotControllerService::subscribe");
        _robotControllerService->subscribe(this);
    }
    else if(msg == "on_close"){
        Logger::trace("web socket connection is closed.");
        Logger::trace("RemoteConnectionService::_videoStream::un_subscribe");
        _videoStream->un_subscribe(this); 
        Logger::trace("RemoteConnectionService::_robotControllerService::un_subscribe");
        _robotControllerService->un_subscribe(this);
    }
    else{
        Json message = Json::parse(msg);
        if (message.contains("talkie")) {
            return;
        }else{
            _robotControllerService->control_motion(message);
        }
    }
}

void RemoteConnectionService::update_sensor_values(nlohmann::json values){
    _sensor_values = values;
}

nlohmann::json RemoteConnectionService::get_sensor_values() {
    return _sensor_values;
}
