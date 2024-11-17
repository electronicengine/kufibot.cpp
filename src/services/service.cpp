#include "service.h"
#include "../ui/main_window.h"

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
        MainWindow::log("Service is starting...", LogLevel::LOG_INFO);
        _serviceThread = std::thread(&Service::service_update_function, this);
    }
}

void Service::stop()
{

    if (_running){
        _running = false;
        MainWindow::log("Service is stopping...", LogLevel::LOG_INFO);
        if (_serviceThread.joinable()) {
            _serviceThread.join(); 
        }
        MainWindow::log("Service is stopped.", LogLevel::LOG_INFO);
    }
}