// VideoStream.h
#ifndef SERVICE_H
#define SERVICE_H

#include <thread>
#include <mutex>
#include <atomic>
#include "../publisher.h"
#include "../subscriber.h"


class Service : public Publisher, public Subscriber{

public:
    Service(const std::string &name);
    virtual ~Service();

    std::string get_service_name();
    virtual void service_function() = 0;
    void run();
    void start();
    void stop();
    void disable();
    void enable();
    bool is_running();

protected:
    std::thread _serviceThread;
    std::mutex _dataMutex;
    std::list<Service*> _subscribedServices;
    std::atomic<bool> _running{false};

    void unsubscribe_all_services();
    void subscribe_to_service(Service *SubscribedService);
    void unsubscribe_from_service(Service *SubscribedService);

private:
    std::string _name;
    std::atomic<bool> _disabled{false};

};

#endif // VIDEOSTREAM_H