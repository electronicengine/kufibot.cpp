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
    std::list<Subscriber*> _subscribers;
    std::mutex _mutex;
    std::string _publisherName;


public:
    Publisher(std::string serviceName) : _publisherName(serviceName) {};
    virtual ~Publisher() = default;


    void subscribe(Subscriber* subscriber);
    void un_subscribe(Subscriber* subscriber);
    void publish(MessageType type, const std::unique_ptr<MessageData>& data);

};


#endif // PUBLISHER_H