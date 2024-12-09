#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <opencv2/opencv.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>


typedef websocketpp::server<websocketpp::config::asio> Server;
using Json = nlohmann::json;

class Subscriber {

public:
    virtual ~Subscriber() = default;

    virtual void update_video_frame(const cv::Mat& frame){ (void)frame; }
    virtual void update_web_socket_message(websocketpp::connection_hdl hdl,  const std::string& mg){ (void) hdl; (void) mg;}
    virtual void update_sensor_values(Json values){(void) values;}

};

#endif // SUBSCRIBER_H