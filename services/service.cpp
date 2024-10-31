#include "service.h"

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
        std::cout << "RobotControllerService is starting..." << std::endl;
        _serviceThread = std::thread(&Service::service_update_function, this);
    }
}

void Service::stop()
{

    if (_running){
        _running = false;
        std::cout << "RobotControllerService is stopping..." << std::endl;
        if (_serviceThread.joinable()) {
            _serviceThread.join(); 
        }
        std::cout << "RobotControllerService is stopped." << std::endl;
    }
}