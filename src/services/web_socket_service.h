
// VideoStream.h
#ifndef WEB_SOCKET_SERVICE_H
#define WEB_SOCKET_SERVICE_H

#include <string>
#include <nlohmann/json.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <condition_variable>
#include "service.h"
#include "../subscriber.h"

#define ASIO_STANDALONE

typedef websocketpp::server<websocketpp::config::asio> Server;

class WebSocketService : public Service {
public:

    static WebSocketService* get_instance();
    virtual ~WebSocketService();

private:
    Server _server;
    websocketpp::connection_hdl _hdl;
    static WebSocketService* _instance;

    WebSocketService();
    void service_function();
    void run_web_server(const std::string& address, uint16_t port);
    void send_message(websocketpp::connection_hdl hdl, const std::string& message);
    void send_data(websocketpp::connection_hdl hdl, const std::vector<uchar>& buffer);
    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, Server::message_ptr msg);

    //subscribed WebSocketTransfer
    void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData> &data);
};

#endif
