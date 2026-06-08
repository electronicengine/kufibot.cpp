#pragma once
#include <string>

class IUdpSocket {
    public:
        virtual ~IUdpSocket() = default;
        virtual bool bindSocket(int port) = 0;
        virtual void setDestination(const std::string& ip, int port) = 0;
        virtual bool sendData(const std::string& data) = 0;
        virtual std::string receiveData(int timeoutMs = 0) = 0;
};

