#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <opencv2/opencv.hpp>
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"
#include "nlohmann/json.hpp"
#include <mutex>

#include "subscriber.h"


typedef websocketpp::server<websocketpp::config::asio> Server;
using Json = nlohmann::json;

class Publisher {

protected:
    std::vector<Subscriber*> _subscribers;  
    std::mutex _mutex;

public:
    virtual ~Publisher() = default;

    void subscribe(Subscriber* subscriber);
    void un_subscribe(Subscriber* subscriber);


    void update_web_socket_message(websocketpp::connection_hdl hdl,  const std::string& msg){
        for (const auto& subscriber : _subscribers) {
            subscriber->update_web_socket_message(hdl, msg); 
        }
    }

    void update_video_frame(const cv::Mat& frame){
        for (const auto& subscriber : _subscribers) {
            subscriber->update_video_frame(frame); 
        }
    }

    void update_sensor_values(Json values){
        for (const auto& subscriber : _subscribers) {
            subscriber->update_sensor_values(values); 
        }
    }

};


#endif // PUBLISHER_H