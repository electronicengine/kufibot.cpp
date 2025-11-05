//
// Created by ybulb on 11/2/2025.
//

#ifndef UDP_SERVER_OPERATOR_H
#define UDP_SERVER_OPERATOR_H


#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

class UdpServerOperator {
public:
    UdpServerOperator();
    ~UdpServerOperator();

    bool openSocket(int port = 0, bool reuse_addr = true);

    void setTarget(const std::string& ip, int port);
    bool sendData(const std::vector<uint8_t>& data);
    bool sendText(const std::string& text);

    void startReceive(std::function<void(const std::vector<uint8_t>&, const std::string&, int)> callback);
    void stopReceive();

    void closeSocket();

private:
    int _sock;
    std::atomic<bool> _running;
    std::mutex _mutex;
    std::thread _recv_thread;
    sockaddr_in _target_addr{};
};

#endif //UDP_SERVER_OPERATOR_H
