#ifndef WEB_SOCKET_OPERATOR_H
#define WEB_SOCKET_OPERATOR_H


#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "operator.h"

class WebSocketOperator : public Operator {
public:
    using Server = websocketpp::server<websocketpp::config::asio>;
    using ConnectionHandle = websocketpp::connection_hdl;
    using OpenHandler = std::function<void(ConnectionHandle)>;
    using CloseHandler = std::function<void(ConnectionHandle)>;
    using MessageHandler = std::function<void(ConnectionHandle, const std::string&)>;

    explicit WebSocketOperator(std::string address = "0.0.0.0", uint16_t port = 8080, int broadcastPort = 8888);
    ~WebSocketOperator() override;

    bool initialize() override;
    void shutdown() override;
    bool isReady() const noexcept override;

    bool start();
    void stop();

    bool sendText(ConnectionHandle hdl, const std::string& message);
    bool sendBinary(ConnectionHandle hdl, const std::vector<unsigned char>& buffer);

    void setOpenHandler(OpenHandler handler);
    void setCloseHandler(CloseHandler handler);
    void setMessageHandler(MessageHandler handler);

private:
    void runServer();
    void runBroadcastResponder();
    void onOpen(ConnectionHandle hdl);
    void onClose(ConnectionHandle hdl);
    void onMessage(ConnectionHandle hdl, Server::message_ptr msg);

    std::string _address;
    uint16_t _port;
    int _broadcastPort;

    mutable std::mutex _serverMutex;
    mutable std::mutex _handlerMutex;
    std::unique_ptr<Server> _server;
    std::thread _serverThread;
    std::thread _broadcastThread;
    std::atomic<bool> _running{false};
    std::atomic<bool> _ready{false};

    OpenHandler _openHandler;
    CloseHandler _closeHandler;
    MessageHandler _messageHandler;
};

#endif // WEB_SOCKET_OPERATOR_H

