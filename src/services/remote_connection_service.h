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
    std::optional<SensorData> _sensorData;
    std::optional<cv::Mat> _frame;
    std::optional<std::string> _socketMessage;
    std::optional<websocketpp::connection_hdl> _hdl;

    static RemoteConnectionService *_instance;


    RemoteConnectionService();

    bool initialize();
    void service_function();
    void speak(std::string text);

    //subscribed video_frame, web_socket_receive_message,sensor_data
    virtual void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data);

};

#endif