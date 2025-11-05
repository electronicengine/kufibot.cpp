//
// Created by ybulb on 11/2/2025.
//

#include "udp_server_operator.h"


UdpServerOperator::UdpServerOperator() : _sock(-1), _running(false) {

}


UdpServerOperator::~UdpServerOperator() {
    closeSocket();
}


bool UdpServerOperator::openSocket(int port, bool reuse_addr) {

    _sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (_sock < 0) {
        perror("socket creation failed");
        return false;
    }

    if (reuse_addr) {
        int opt = 1;
        setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }

    if (port > 0) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (bind(_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
            perror("bind failed");
            closeSocket();
            return false;
        }
    }

    return true;
}
void UdpServerOperator::setTarget(const std::string &ip, int port) {
    std::lock_guard<std::mutex> lock(_mutex);
    memset(&_target_addr, 0, sizeof(_target_addr));
    _target_addr.sin_family = AF_INET;
    _target_addr.sin_port = htons(port);
    inet_aton(ip.c_str(), &_target_addr.sin_addr);
}


bool UdpServerOperator::sendData(const std::vector<uint8_t> &data) {

    std::lock_guard<std::mutex> lock(_mutex);
    if (_sock < 0)
        return false;

    ssize_t sent = sendto(_sock, data.data(), data.size(), 0, (struct sockaddr *) &_target_addr, sizeof(_target_addr));
    return sent >= 0;
}


bool UdpServerOperator::sendText(const std::string &text) {
    return sendData(std::vector<uint8_t>(text.begin(), text.end()));
}


void UdpServerOperator::startReceive(
        std::function<void(const std::vector<uint8_t> &, const std::string &, int)> callback) {

    if (_running)
        return;

    _running = true;
    _recv_thread = std::thread([this, callback]() {
        sockaddr_in sender_addr{};
        socklen_t sender_len = sizeof(sender_addr);
        std::vector<uint8_t> buffer(65535);

        while (_running) {
            ssize_t received =
                    recvfrom(_sock, buffer.data(), buffer.size(), 0, (struct sockaddr *) &sender_addr, &sender_len);
            if (received > 0) {
                buffer.resize(received);
                std::string sender_ip = inet_ntoa(sender_addr.sin_addr);
                int sender_port = ntohs(sender_addr.sin_port);
                callback(buffer, sender_ip, sender_port);
                buffer.resize(65535); // reset buffer size
            }
        }
    });
}


void UdpServerOperator::stopReceive() {

    _running = false;
    if (_recv_thread.joinable())
        _recv_thread.join();
}


void UdpServerOperator::closeSocket() {

    stopReceive();
    if (_sock >= 0) {
        close(_sock);
        _sock = -1;
    }

}
