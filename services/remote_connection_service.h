#ifndef REMOTE_CONNECTION_SERVICE_H
#define REMOTE_CONNECTION_SERVICE_H


#include <opencv2/opencv.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include <thread>
#include <atomic>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <nlohmann/json.hpp>
#include <iostream>
#include <functional>
#include <map> 
#include <unordered_map>
#include <chrono>
#include <thread>

#include "service.h"
#include "../subscriber.h"
#include "web_socket_service.h"
#include "video_stream_service.h"
#include "robot_controller_service.h"


using Json = nlohmann::json;

class RemoteConnectionService : public Subscriber, public Service{

private:
    int _port;
    std::string _ip;
    WebSocketService *_webSocket;
    VideoStreamService *_videoStream;
    RobotControllerService *_robotControllerService;
    websocketpp::connection_hdl _hdl;
    Json _sensor_values;
    static RemoteConnectionService *_instance;

    RemoteConnectionService(int port = 8765);

public:

    static RemoteConnectionService *get_instance();
    ~RemoteConnectionService();

    void start();
    void stop(); 
    void service_update_function(){}
    void update_video_frame(const cv::Mat& frame);
    void update_web_socket_message(websocketpp::connection_hdl hdl,  const std::string& mg);
    void update_sensor_values(Json values);


};

#endif