#include "remote_connection_service.h"


RemoteConnectionService* RemoteConnectionService::_instance = nullptr;


RemoteConnectionService *RemoteConnectionService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new RemoteConnectionService();
    }
    return _instance;
}

RemoteConnectionService::RemoteConnectionService(int port)
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

}


void RemoteConnectionService::stop() {
    std::cout << "RemoteConnectionService is stopping..." << std::endl;

    std::cout << "RemoteConnectionService::_webSocket::un_subscribe" << std::endl;
    _webSocket->un_subscribe(this); 
    std::cout << "RemoteConnectionService::_videoStream::un_subscribe" << std::endl;
    _videoStream->un_subscribe(this); 
    std::cout << "RemoteConnectionService::_robotControllerService::un_subscribe" << std::endl;
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
        std::cerr << "WebSocket Error: " << e.what() << std::endl;
    }

}

void RemoteConnectionService::update_web_socket_message(websocketpp::connection_hdl hdl,  const std::string& msg)
{
    if(msg == "on_open"){
        std::cout << "new web socket connection extablished" << std::endl;
        _hdl = hdl;
        std::cout << "RemoteConnectionService::_videoStream::subscribe" << std::endl;
        _videoStream->subscribe(this);
        std::cout << "RemoteConnectionService::_robotControllerService::subscribe" << std::endl;
        _robotControllerService->subscribe(this);

    }
    else if(msg == "on_close"){
        std::cout << "web socket connection is closed." << std::endl;
        std::cout << "RemoteConnectionService::_videoStream::un_subscribe" << std::endl;
        _videoStream->un_subscribe(this); 
        std::cout << "RemoteConnectionService::_robotControllerService::un_subscribe" << std::endl;
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