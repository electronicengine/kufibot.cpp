#ifndef CONTROLLER_DATA_STRUCTURES_H
#define CONTROLLER_DATA_STRUCTURES_H

#include <stdint.h>
#include <map>
#include <string>

enum class DCMotorDirection {
    forward,
    backward
};

enum class DCMotor {
    left,
    right
};

struct DCMotorState {
    int leftMotorMagnitude;
    DCMotorDirection leftMotorDirection;
    int rightMotorMagnitude;
    DCMotorDirection rightMotorDirection;
};


struct CompassData {
    uint16_t angle;
    int16_t magnetX;
    int16_t magnetY;
};

struct DistanceData {
    int distance;
    int strength;
    int temperature;
};

struct PowerData {
    float busVoltage;
    float shuntVoltage;
    float current;
    float power;
};


enum class ServoMotorJoint {
    rightArm = 0,
    leftArm,
    neck,
    headUpDown,
    headLeftRight,
    eyeRight,
    eyeLeft,
};

inline const std::map<ServoMotorJoint, std::string> Servo_Motor_Joint_Names = {{ServoMotorJoint::rightArm, "Right Arm"},
                                                                    {ServoMotorJoint::leftArm, "Left Arm"},
                                                                    {ServoMotorJoint::neck, "Neck"},
                                                                    {ServoMotorJoint::headUpDown, "Head Up-Down"},
                                                                    {ServoMotorJoint::headLeftRight, "Head Left-Right"},
                                                                    {ServoMotorJoint::eyeRight, "Eye Right"},
                                                                    {ServoMotorJoint::eyeLeft, "Eye Left"}};


inline const std::map<ServoMotorJoint, uint8_t> Default_Joint_Angles = {
    {ServoMotorJoint::rightArm, 15}, {ServoMotorJoint::leftArm, 170}, {ServoMotorJoint::neck, 78},
    {ServoMotorJoint::headUpDown, 15}, {ServoMotorJoint::headLeftRight, 90}, {ServoMotorJoint::eyeRight, 160},
    {ServoMotorJoint::eyeLeft, 20}
};

#endif //CONTROLLER_DATA_STRUCTURES_H
