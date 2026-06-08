#pragma once

#include "IUdpSocket.h"
#include <string>
#include <vector>
#include <netinet/in.h>


class UdpServerSocket : public IUdpSocket {
public:
    UdpServerSocket();
    ~UdpServerSocket() override;

    bool bindSocket(int port) override;
    void setDestination(const std::string& ip, int port) override;

    bool sendData(const std::string& data) override;
    std::string receiveData(int timeoutMs = 0) override;

private:
    int sockfd_;
    struct sockaddr_in destAddr_;
    bool destSet_;
    struct sockaddr_in lastClientAddr_;
    bool hasLastClient_;
};

