
// VideoStream.h
#ifndef WEB_SOCKET_SERVICE_H
#define WEB_SOCKET_SERVICE_H

#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <boost/asio/ip/address.hpp> 
#include <boost/asio/ip/tcp.hpp> 
#include <functional>
#include <csignal>
#include <mutex>
#include <condition_variable>
#include <queue>

#include "../publisher.h"

#define ASIO_STANDALONE

typedef websocketpp::server<websocketpp::config::asio> Server;

class WebSocketService : public Publisher {
public:

    static WebSocketService* get_instance();
    void run(const std::string& address, uint16_t port);
    void send_message(websocketpp::connection_hdl hdl, const std::string& message);
    void send_data(websocketpp::connection_hdl hdl, const std::vector<uchar>& buffer);

    void start(const std::string& address, uint16_t port); 
    void stop();                

private:
    Server _server;
    websocketpp::connection_hdl _hdl;
    std::thread _serverThread; 
    std::atomic<bool> _running{false}; 
    static WebSocketService* _instance;

    WebSocketService();
    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, Server::message_ptr msg);

};

#endif
