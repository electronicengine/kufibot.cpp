// VideoStream.h
#ifndef SERVICE_H
#define SERVICE_H

#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include "../publisher.h"

class Service {
protected:
    std::thread _serviceThread;
    std::atomic<bool> _running{false}; 
    std::string _name;
    std::mutex _dataMutex; 

public:

    Service(const std::string &name);
    ~Service();  

    std::string get_service_name();
    virtual void service_update_function() = 0;
    virtual void start();  
    virtual void stop();  
};

#endif // VIDEOSTREAM_H