#include "web_socket_operator.h"

#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "../logger.h"

WebSocketOperator::WebSocketOperator(std::string address, uint16_t port, int broadcastPort)
    : Operator("WebSocketOperator"),
      _address(std::move(address)),
      _port(port),
      _broadcastPort(broadcastPort) {
}

WebSocketOperator::~WebSocketOperator() {
    stop();
}

bool WebSocketOperator::initialize() {
    return true;
}

void WebSocketOperator::shutdown() {
    stop();
}

bool WebSocketOperator::isReady() const noexcept {
    return _ready.load();
}

bool WebSocketOperator::start() {
    bool expected = false;
    if (!_running.compare_exchange_strong(expected, true)) {
        return true;
    }

    {
        std::lock_guard<std::mutex> lock(_serverMutex);
        _server = std::make_unique<Server>();
        _server->init_asio();
        _server->clear_access_channels(websocketpp::log::alevel::all);
        _server->set_reuse_addr(true);
        _server->set_open_handler([this](ConnectionHandle hdl) { onOpen(hdl); });
        _server->set_close_handler([this](ConnectionHandle hdl) { onClose(hdl); });
        _server->set_message_handler([this](ConnectionHandle hdl, Server::message_ptr msg) { onMessage(hdl, std::move(msg)); });
    }

    _broadcastThread = std::thread(&WebSocketOperator::runBroadcastResponder, this);
    _serverThread = std::thread(&WebSocketOperator::runServer, this);
    return true;
}

void WebSocketOperator::stop() {
    bool expected = true;
    if (!_running.compare_exchange_strong(expected, false)) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(_serverMutex);
        if (_server != nullptr) {
            websocketpp::lib::error_code ec;
            _server->stop_listening(ec);
            _server->stop();
        }
    }

    if (_serverThread.joinable()) {
        _serverThread.join();
    }

    if (_broadcastThread.joinable()) {
        _broadcastThread.join();
    }

    std::lock_guard<std::mutex> lock(_serverMutex);
    _server.reset();
    _ready = false;
}

bool WebSocketOperator::sendText(ConnectionHandle hdl, const std::string& message) {
    std::lock_guard<std::mutex> lock(_serverMutex);
    if (_server == nullptr || !_ready.load()) {
        return false;
    }

    try {
        _server->send(hdl, message, websocketpp::frame::opcode::text);
        return true;
    } catch (const websocketpp::exception& e) {
        INFO("Send failed: {}", e.what());
        return false;
    }
}

bool WebSocketOperator::sendBinary(ConnectionHandle hdl, const std::vector<unsigned char>& buffer) {
    std::lock_guard<std::mutex> lock(_serverMutex);
    if (_server == nullptr || !_ready.load()) {
        return false;
    }

    try {
        _server->send(hdl, buffer.data(), buffer.size(), websocketpp::frame::opcode::binary);
        return true;
    } catch (const websocketpp::exception& e) {
        INFO("Send failed: {}", e.what());
        return false;
    }
}

void WebSocketOperator::setOpenHandler(OpenHandler handler) {
    std::lock_guard<std::mutex> lock(_handlerMutex);
    _openHandler = std::move(handler);
}

void WebSocketOperator::setCloseHandler(CloseHandler handler) {
    std::lock_guard<std::mutex> lock(_handlerMutex);
    _closeHandler = std::move(handler);
}

void WebSocketOperator::setMessageHandler(MessageHandler handler) {
    std::lock_guard<std::mutex> lock(_handlerMutex);
    _messageHandler = std::move(handler);
}

void WebSocketOperator::runServer() {
    try {
        {
            std::lock_guard<std::mutex> lock(_serverMutex);
            if (_server == nullptr) {
                return;
            }

            _server->listen(_port);
            _server->start_accept();
        }

        INFO("WebSocket server listening on {}:{}", _address, _port);
        _ready = true;

        {
            std::lock_guard<std::mutex> lock(_serverMutex);
            if (_server != nullptr) {
                _server->run();
            }
        }
    } catch (const websocketpp::exception& e) {
        ERROR("WebSocket++ exception: {}", e.what());
    } catch (...) {
        ERROR("Other exception occurred in WebSocketOperator");
    }

    _ready = false;
}

void WebSocketOperator::runBroadcastResponder() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        ERROR("Failed to create broadcast socket");
        return;
    }

    timeval timeout{};
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    int broadcastEnable = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(_broadcastPort);

    if (bind(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
        ERROR("Failed to bind broadcast socket");
        close(sock);
        return;
    }

    INFO("Broadcast responder listening on port {}", _broadcastPort);

    char buffer[1024];
    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);

    while (_running.load()) {
        const int received = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                                      reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);

        if (received <= 0) {
            continue;
        }

        buffer[received] = '\0';
        const std::string message(buffer);
        INFO("Received discovery request: {} from {}", message, inet_ntoa(clientAddr.sin_addr));

        if (message == "DISCOVER_WEBSOCKET_SERVER") {
            const std::string response = "WEBSOCKET_SERVER:" + std::to_string(_port);
            sendto(sock, response.c_str(), response.length(), 0,
                   reinterpret_cast<sockaddr*>(&clientAddr), clientLen);
            INFO("Sent response to {}", inet_ntoa(clientAddr.sin_addr));
        }
    }

    close(sock);
}

void WebSocketOperator::onOpen(ConnectionHandle hdl) {
    WARNING("Connection opened!");

    OpenHandler handler;
    {
        std::lock_guard<std::mutex> lock(_handlerMutex);
        handler = _openHandler;
    }

    if (handler) {
        handler(hdl);
    }
}

void WebSocketOperator::onClose(ConnectionHandle hdl) {
    WARNING("Connection closed!");

    CloseHandler handler;
    {
        std::lock_guard<std::mutex> lock(_handlerMutex);
        handler = _closeHandler;
    }

    if (handler) {
        handler(hdl);
    }
}

void WebSocketOperator::onMessage(ConnectionHandle hdl, Server::message_ptr msg) {
    MessageHandler handler;
    {
        std::lock_guard<std::mutex> lock(_handlerMutex);
        handler = _messageHandler;
    }

    if (handler) {
        handler(hdl, msg->get_payload());
    }
}

