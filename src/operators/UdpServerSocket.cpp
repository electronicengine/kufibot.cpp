#include "UdpServerSocket.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <sys/time.h>
#include <iostream>
#include "../logger.h"


UdpServerSocket::UdpServerSocket() : sockfd_(-1), destSet_(false), hasLastClient_(false) {
    std::memset(&destAddr_, 0, sizeof(destAddr_));
    std::memset(&lastClientAddr_, 0, sizeof(lastClientAddr_));
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) {
        ERROR("Failed to create UDP server socket.");
    }
}

UdpServerSocket::~UdpServerSocket() {
    if (sockfd_ >= 0) {
        close(sockfd_);
    }
}

bool UdpServerSocket::bindSocket(int port) {
    if (sockfd_ < 0) return false;

    struct sockaddr_in servAddr;
    std::memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(port);

    if (bind(sockfd_, (const struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        ERROR("Failed to bind UDP server socket to port {}", port);
        return false;
    }
    return true;
}

void UdpServerSocket::setDestination(const std::string& ip, int port) {
    std::memset(&destAddr_, 0, sizeof(destAddr_));
    destAddr_.sin_family = AF_INET;
    destAddr_.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &destAddr_.sin_addr);
    destSet_ = true;
}

bool UdpServerSocket::sendData(const std::string& data) {
    if (sockfd_ < 0) return false;

    struct sockaddr_in* target = nullptr;
    if (destSet_) {
        target = &destAddr_;
    } else if (hasLastClient_) {
        target = &lastClientAddr_;
    }

    if (!target) return false;

    ssize_t sentBytes = sendto(sockfd_, data.c_str(), data.size(),
        0, (const struct sockaddr *)target, sizeof(struct sockaddr_in));
    return sentBytes == static_cast<ssize_t>(data.size());
}

std::string UdpServerSocket::receiveData(int timeoutMs) {
    if (sockfd_ < 0) return "";

    if (timeoutMs > 0) {
        struct timeval tv;
        tv.tv_sec = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs % 1000) * 1000;
        setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    } else {
        struct timeval tv = {0, 0};
        setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }

    char buffer[2048];
    socklen_t len = sizeof(lastClientAddr_);

    ssize_t n = recvfrom(sockfd_, buffer, sizeof(buffer) - 1,
        0, (struct sockaddr *)&lastClientAddr_, &len);

    if (n > 0) {
        buffer[n] = '\0';
        hasLastClient_ = true;
        return std::string(buffer);
    }
    return "";
}

