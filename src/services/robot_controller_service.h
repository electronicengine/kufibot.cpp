#ifndef ROBOT_CONTROLLER_SERVICE_H
#define ROBOT_CONTROLLER_SERVICE_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <thread>
#include <atomic>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iostream>
#include <functional>
#include <map> 
#include <unordered_map>
#include <chrono>
#include <thread>

#include "service.h"
#include "../subscriber.h"
#include "../publisher.h"
#include "../controllers/compass_controller.h"
#include "../controllers/distance_controller.h"
#include "../controllers/power_controller.h"
#include "../controllers/servo_motor_controller.h"
#include "../controllers/dc_motor_controller.h"

using Json = nlohmann::json;

enum State{
    stop,
    idle,
    move
};

class RobotControllerService : public Publisher, public Service{

private:

    Json _sensor_values;
    State _current_state{State::stop};

    CompassController *_compassController;
    DistanceController *_distanceController;
    PowerController *_powerController;
    ServoMotorController *_servoController; 
    DCMotorController *_dcMotorController;

    void update_sensors(); 
    static RobotControllerService* _instance;
    RobotControllerService();


public:

    ~RobotControllerService();

    static RobotControllerService *get_instance();
    Json get_sensor_values();
    std::map<std::string, int> get_servo_joint_map();
    void set_servo_joint_map(const std::map<std::string, int>& jointMap);

    int control_motion(Json message);
    void control_body(int angle, int magnitude);
    void control_head(int angle, int magnitude);
    void control_arm(const std::string& control_id, int angle);
    void control_eye(int angle);
    void service_update_function();
    void start();
    void stop(); 


};

#endif