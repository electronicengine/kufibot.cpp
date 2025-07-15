#include "service.h"
#include "../logger.h"

Service::Service(const std::string &name) : _name(name), Publisher(name)
{
}

Service::~Service()
{
}

std::string Service::get_service_name()
{
    return _name;
}

void Service::unsubscribe_all_services() {
    for (auto service : _subscribedServices) {
        Logger::info("{} is unsubscribing from {}", _name.c_str(), service->get_service_name().c_str());
        service->un_subscribe(this);
    }
    _subscribedServices.clear();
}

void Service::subscribe_to_service(Service *subscribedService) {
    Logger::info("{} is subscribing to {}", _name.c_str(), subscribedService->get_service_name().c_str());
    subscribedService->subscribe(this);
    _subscribedServices.push_back(subscribedService);
}

void Service::unsubscribe_from_service(Service *SubscribedService) {
    Logger::info("{} is unsubscribing from {}", _name.c_str(), SubscribedService->get_service_name().c_str());
    SubscribedService->un_subscribe(this);
    _subscribedServices.erase(std::remove(_subscribedServices.begin(), _subscribedServices.end(), SubscribedService), _subscribedServices.end());
}


void Service::start()
{
    if (!_disabled) {
        if (!_running) {
            _running = true;
            Logger::info("{} is starting...", _name.c_str());
            _serviceThread = std::thread(&Service::service_function, this);
        }
    }else {
        Logger::warn("{} is not starting... Because, it is disabled.", _name.c_str());
    }
}

void Service::stop()
{
    if (_running){
        _running = false;
        unsubscribe_all_services();
        Logger::info("{} is stopping...", _name.c_str());
        if (_serviceThread.joinable()) {
            _serviceThread.join(); 
        }
        Logger::info("{} is stopped.", _name.c_str());
    }
}

void Service::disable() {
    if (_running) {
        Logger::warn("{} is not disabling... Because, it is running. To disable, stop it first!", _name.c_str());
        return;
    }else {
        Logger::warn("{} is not disabling...", _name.c_str());
        _disabled = true;
    }
}

void Service::enable() {
    Logger::warn("{} is enabling...", _name.c_str());

    _disabled = false;
}
