#ifndef SERVO_MOTOR_CONTROLLER_H
#define SERVO_MOTOR_CONTROLLER_H

#include <string>
#include <map>
#include <nlohmann/json.hpp>

#include "../drivers/pca9685_driver.h"

using Json = nlohmann::json;


enum class ServoMotorJoint {
    rightArm = 0,
    leftArm,
    neck,
    headUpDown,
    headLeftRight,
    eyeRight,
    eyeLeft,
};

std::map<ServoMotorJoint, std::string> Servo_Motor_Joint_Names = {{ServoMotorJoint::rightArm, "Right Arm"},
                                                                    {ServoMotorJoint::leftArm, "Left Arm"},
                                                                    {ServoMotorJoint::neck, "Neck"},
                                                                    {ServoMotorJoint::headUpDown, "Head Up-Down"},
                                                                    {ServoMotorJoint::headLeftRight, "Head Left-Right"},
                                                                    {ServoMotorJoint::eyeRight, "Eye Right"},
                                                                    {ServoMotorJoint::eyeLeft, "Eye Left"}};

class ServoMotorController {
public:
    static ServoMotorController *get_instance(int address = 0x40);
    void save_joint_angles();
    void load_joint_angles();
    void set_absolute_servo_angle(ServoMotorJoint joint, int targetAngle, int step = 1, int delayMs = 10);
    std::map<ServoMotorJoint, int> get_current_joint_angles();
    void set_current_joint_angles(std::map<ServoMotorJoint, int>& angles);

    void head_down();
    void head_up();
    void head_left();
    void head_right();
    void eye_up();
    void eye_down();
    void eye_angry();
    void eye_wondering();

private:
    ServoMotorController(int address);

    static ServoMotorController* _instance;
    PCA9685Driver _driver;
    std::map<ServoMotorJoint, int> _currentJointAngles;
};

#endif