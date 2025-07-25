#include "service.h"
#include "../logger.h"

Service::Service(const std::string &name) : Publisher(name), _name(name)
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
        INFO("{} is unsubscribing from {}", _name.c_str(), service->get_service_name().c_str());
        service->un_subscribe(this);
    }
    _subscribedServices.clear();
}

void Service::subscribe_to_service(Service *subscribedService) {
    INFO("{} is subscribing to {}", _name.c_str(), subscribedService->get_service_name().c_str());
    subscribedService->subscribe(this);
    _subscribedServices.push_back(subscribedService);
}

void Service::unsubscribe_from_service(Service *SubscribedService) {
    INFO("{} is unsubscribing from {}", _name.c_str(), SubscribedService->get_service_name().c_str());
    SubscribedService->un_subscribe(this);
    _subscribedServices.erase(std::remove(_subscribedServices.begin(), _subscribedServices.end(), SubscribedService), _subscribedServices.end());
}


void Service::start()
{
    if (!_disabled) {
        if (!_running) {
            _running = true;
            INFO("{} is starting...", _name.c_str());
            _serviceThread = std::thread(&Service::service_function, this);
        }
    }else {
        WARNING("{} is not starting... Because, it is disabled.", _name.c_str());
    }
}

void Service::run()
{
    if (!_disabled) {
        if (!_running) {
            _running = true;
            INFO("{} is running...", _name.c_str());
            service_function();
        }
    }else {
        WARNING("{} is not running... Because, it is disabled.", _name.c_str());
    }
}

void Service::stop()
{
    if (_running){
        _running = false;
        unsubscribe_all_services();
        INFO("{} is stopping...", _name.c_str());
        if (_serviceThread.joinable()) {
            _serviceThread.join(); 
        }
        INFO("{} is stopped.", _name.c_str());
    }
}

void Service::disable() {
    if (_running) {
        WARNING("{} could not disabled! Because, it is running. To disable, stop it first!", _name.c_str());
        return;
    }else {
        WARNING("{} is disabled!", _name.c_str());
        _disabled = true;
    }
}

void Service::enable() {
    WARNING("{} is enabling...", _name.c_str());

    _disabled = false;
}

bool Service::is_running() {
    return _running;
}
