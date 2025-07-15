#ifndef ROBOT_CONTROLLER_SERVICE_H
#define ROBOT_CONTROLLER_SERVICE_H

#include <string>
#include <map> 


#include "service.h"
#include "../subscriber.h"
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

class RobotControllerService : public Service{

public:

    virtual ~RobotControllerService();

    static RobotControllerService *get_instance();

private:

    Json _sensor_values;
    State _current_state{State::stop};

    CompassController *_compassController;
    DistanceController *_distanceController;
    PowerController *_powerController;
    ServoMotorController *_servoController;
    DCMotorController *_dcMotorController;

    static RobotControllerService* _instance;

    RobotControllerService();

    void service_function();

    void update_sensors();
    Json get_sensor_values();
    std::map<std::string, int> get_servo_joint_map();
    void set_servo_joint_map(const std::map<std::string, int>& jointMap);

    int control_motion(Json message);
    void control_body(int angle, int magnitude);
    void control_head(int angle, int magnitude);
    void control_arm(const std::string& control_id, int angle, bool scale = true);
    void set_all_joint_angles(const std::map<std::string, int>& angles);
    void control_eye(int angle);

    //subscibed control_data
    virtual void subcribed_data_receive(MessageType type, MessageData* data);


};

#endif