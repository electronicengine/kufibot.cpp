#ifndef ROBOT_CONTROLLER_SERVICE_H
#define ROBOT_CONTROLLER_SERVICE_H

#include <string>
#include <map> 


#include "service.h"
#include "../subscriber.h"
#include "../statemachines/robot.h"



class RobotControllerService : public Service{

public:

    virtual ~RobotControllerService();

    static RobotControllerService *get_instance();

private:

    Json _sensor_values;

    static RobotControllerService* _instance;
    Robot _robot;

    RobotControllerService();

    void service_function();

    //subscibed control_data
    virtual void subcribed_data_receive(MessageType type,  const std::unique_ptr<MessageData>& data);


};

#endif