#ifndef SERVO_MOTOR_CONTROLLER_H
#define SERVO_MOTOR_CONTROLLER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <unistd.h>
#include <map>
#include <nlohmann/json.hpp>

#include "../drivers/pca9685_driver.h"

using Json = nlohmann::json;

class ServoMotorController {
public:
    static ServoMotorController *get_instance(int address = 0x40);
    void save_joint_angles();
    void load_joint_angles();
    void set_all_angles(const std::map<std::string, int>& angles);
    void set_absolute_servo_angle(const std::string& joint, int targetAngle, int step = 1, int delayMs = 10);
    const std::map<std::string, int>& get_all_joint_angles();
    std::map<std::string, int> get_joint_channels();
    void head_down();
    void head_up();
    void head_left();
    void head_right();
    void eye_up();
    void eye_down();

private:
    ServoMotorController(int address);

    static ServoMotorController* _instance;
    PCA9685Driver _driver;
    std::map<std::string, int> _jointChannels;
    std::map<std::string, int> _jointAngles;
};

#endif