#ifndef REMOTE_CONNECTION_SERVICE_H
#define REMOTE_CONNECTION_SERVICE_H


#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>
#include <string>


#include "service.h"
#include "../subscriber.h"
#include "web_socket_service.h"
#include "video_stream_service.h"
#include "robot_controller_service.h"



class RemoteConnectionService : public Service{


public:

    static RemoteConnectionService *get_instance();
    virtual ~RemoteConnectionService();

private:
    int _port;
    std::string _ip;
    WebSocketService *_webSocketService;
    VideoStreamService *_videoStreamService;
    RobotControllerService *_robotControllerService;
    websocketpp::connection_hdl _hdl;
    SensorData _sensorData;
    static RemoteConnectionService *_instance;


    RemoteConnectionService(int port = 8765);
    void service_function();

    //subscribed video_frame, web_socket_receive_message,sensor_data
    virtual void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data);

    void video_frame(const cv::Mat& frame);
    void web_socket_receive_message(websocketpp::connection_hdl hdl,  const std::string& msg);


};

#endif