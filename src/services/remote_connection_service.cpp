#include "remote_connection_service.h"
#include "../ui/main_window.h"

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
    MainWindow::log("RemoteConnectionService is starting...", LogLevel::LOG_INFO);
}


void RemoteConnectionService::stop() {
    MainWindow::log("RemoteConnectionService is stopping...", LogLevel::LOG_INFO);

    MainWindow::log("RemoteConnectionService::_webSocket::un_subscribe", LogLevel::LOG_TRACE);
    _webSocket->un_subscribe(this); 
    MainWindow::log("RemoteConnectionService::_videoStream::un_subscribe", LogLevel::LOG_TRACE);
    _videoStream->un_subscribe(this); 
    MainWindow::log("RemoteConnectionService::_robotControllerService::un_subscribe", LogLevel::LOG_TRACE);
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
        MainWindow::log("WebSocket Error: " + std::string(e.what()), LogLevel::LOG_ERROR);
    }

}

void RemoteConnectionService::update_web_socket_message(websocketpp::connection_hdl hdl,  const std::string& msg)
{
    if(msg == "on_open"){
        MainWindow::log("new web socket connection extablished", LogLevel::LOG_TRACE);
        _hdl = hdl;
        MainWindow::log("RemoteConnectionService::_videoStream::subscribe", LogLevel::LOG_TRACE);
        _videoStream->subscribe(this);
        MainWindow::log("RemoteConnectionService::_robotControllerService::subscribe", LogLevel::LOG_TRACE);
        _robotControllerService->subscribe(this);
    }
    else if(msg == "on_close"){
        MainWindow::log("web socket connection is closed.", LogLevel::LOG_TRACE);
        MainWindow::log("RemoteConnectionService::_videoStream::un_subscribe", LogLevel::LOG_TRACE);
        _videoStream->un_subscribe(this); 
        MainWindow::log("RemoteConnectionService::_robotControllerService::un_subscribe", LogLevel::LOG_TRACE);
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

void RemoteConnectionService::update_sensor_values(Json values){
    _sensor_values = values;
}