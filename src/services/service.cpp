#include "service.h"
#include "../logger.h"

Service::Service(const std::string &name) : _name(name)
{
}

Service::~Service()
{
}

std::string Service::get_service_name()
{
    return _name;
}

void Service::start()
{
    if (!_running) { 
        _running = true;
        Logger::info("Service is starting...");
        _serviceThread = std::thread(&Service::service_update_function, this);
    }
}

void Service::stop()
{
    if (_running){
        _running = false;
        Logger::info("Service is stopping...");
        if (_serviceThread.joinable()) {
            _serviceThread.join(); 
        }
        Logger::info("Service is stopped.");
    }
}