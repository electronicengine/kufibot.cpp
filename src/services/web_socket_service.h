
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
#include "../logger.h"

#define ASIO_STANDALONE

class BroadcastResponder {
private:
    int broadcast_port = 8888;
    int websocket_port = 8080;
    bool running = false;
    std::thread listener_thread;

public:
    void start() {
        running = true;
        listener_thread = std::thread(&BroadcastResponder::listen, this);
    }

    void stop() {
        running = false;
        if (listener_thread.joinable()) {
            listener_thread.join();
        }
    }

private:
    void listen() {
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            ERROR("Failed to create broadcast socket");
            return;
        }

        // Broadcast'e izin ver
        int broadcast_enable = 1;
        setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));

        // Port'a bindq
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(broadcast_port);

        if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            ERROR("Failed to bind broadcast socket");
            close(sock);
            return;
        }

        INFO("Broadcast responder listening on port {}", broadcast_port);

        char buffer[1024];
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        while (running) {
            int received = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                                   (struct sockaddr*)&client_addr, &client_len);

            if (received > 0) {
                buffer[received] = '\0';
                std::string message(buffer);

                INFO("Received discovery request: {} from {}",
                     message, inet_ntoa(client_addr.sin_addr));

                if (message == "DISCOVER_WEBSOCKET_SERVER") {
                    // Yanıt gönder
                    std::string response = "WEBSOCKET_SERVER:" + std::to_string(websocket_port);
                    sendto(sock, response.c_str(), response.length(), 0,
                          (struct sockaddr*)&client_addr, client_len);

                    INFO("Sent response to {}", inet_ntoa(client_addr.sin_addr));
                }
            }
        }

        close(sock);
    }
};


typedef websocketpp::server<websocketpp::config::asio> Server;

class WebSocketService : public Service {
public:

    static WebSocketService* get_instance();
    virtual ~WebSocketService();

private:
    Server _server;
    websocketpp::connection_hdl _hdl;
    static WebSocketService* _instance;
    std::unique_ptr<BroadcastResponder> _broadcastResponder;
    WebSocketService();
    void service_function();
    void run_web_server(const std::string& address, uint16_t port);
    void send_message(websocketpp::connection_hdl hdl, const std::string& message);
    void send_data(websocketpp::connection_hdl hdl, const std::vector<uchar>& buffer);
    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, Server::message_ptr msg);

    bool initialize();
    void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData> &data);
};

#endif
