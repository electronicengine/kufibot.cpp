#ifndef SERVO_MOTOR_CONTROLLER_H
#define SERVO_MOTOR_CONTROLLER_H

#include <map>
#include <nlohmann/json.hpp>
#include "controller_data_structures.h"
#include "../drivers/pca9685_driver.h"

using Json = nlohmann::json;


class ServoMotorController {
public:
    static ServoMotorController *get_instance(int address = 0x40);
    void save_joint_angles();
    void load_joint_angles();
    void set_absolute_servo_angle(ServoMotorJoint joint, int targetAngle, int step = 1, int delayMs = 10);
    std::map<ServoMotorJoint, uint8_t> get_current_joint_angles();
    void set_current_joint_angles(std::map<ServoMotorJoint, uint8_t>& angles);


private:
    ServoMotorController(int address);

    static ServoMotorController* _instance;
    PCA9685Driver _driver;
    std::map<ServoMotorJoint, uint8_t> _currentJointAngles;
};

#endif