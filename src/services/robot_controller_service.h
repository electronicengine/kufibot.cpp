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
    SensorData get_sensor_values();

    void control_motion(const ControlData& controlData);
    void control_body(int angle, int magnitude);
    void control_head(int angle, int magnitude);
    void control_arm(ServoMotorJoint joint, int angle, bool scale = true) const;
    void set_all_joint_angles(const std::map<std::string, int>& angles);
    void control_eye(ServoMotorJoint joint, bool state);

    void head_down();
    void head_up();
    void head_left();
    void head_right();
    void eye_up();
    void eye_down();

    //subscibed control_data
    virtual void subcribed_data_receive(MessageType type,  const std::unique_ptr<MessageData>& data);


};

#endif