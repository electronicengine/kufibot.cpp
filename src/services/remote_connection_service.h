#ifndef REMOTE_CONNECTION_SERVICE_H
#define REMOTE_CONNECTION_SERVICE_H


#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>
#include <string>


#include "../operators/web_socket_operator.h"
#include "service.h"
#include "robot_controller_service.h"



class RemoteConnectionService : public Service{


public:

    static RemoteConnectionService *get_instance();
    ~RemoteConnectionService() override;

private:
    using ConnectionHandle = WebSocketOperator::ConnectionHandle;

    int _port;
    std::string _ip;
    std::optional<SensorData> _sensorData;
    std::optional<cv::Mat> _frame;
    std::optional<std::string> _socketMessage;
    std::optional<ConnectionHandle> _hdl;
    WebSocketOperator _webSocketOperator;

    static RemoteConnectionService *_instance;


    RemoteConnectionService();

    bool initialize() override;
    void service_function() override;
    void speak(std::string text);
    void handle_websocket_open(const ConnectionHandle& hdl);
    void handle_websocket_close(const ConnectionHandle& hdl);
    void handle_websocket_message(const ConnectionHandle& hdl, const std::string& message);

    //subscribed video_frame,sensor_data
    void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data) override;

};

#endif