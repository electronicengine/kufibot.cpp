#include "servo_motor_controller.h"
#include "../logger.h"
#include <fstream>

ServoMotorController* ServoMotorController::_instance = nullptr;

ServoMotorController *ServoMotorController::get_instance(int address)
{
    if (_instance == nullptr) {
        _instance = new ServoMotorController(address);
    }
    return _instance;
}

ServoMotorController::ServoMotorController(int address) {

    _driver = PCA9685Driver(address, false);
    _driver.set_pwm_freq(50);

    _currentJointAngles = {
        {ServoMotorJoint::rightArm, 15}, {ServoMotorJoint::leftArm, 170}, {ServoMotorJoint::neck, 78},
        {ServoMotorJoint::headUpDown, 15}, {ServoMotorJoint::headLeftRight, 90}, {ServoMotorJoint::eyeRight, 160},
        {ServoMotorJoint::eyeLeft, 20}
    };

    Logger::info("Servo driver initialized");
}

void ServoMotorController::save_joint_angles()
{
    // std::ofstream outFile("joint_angles.json");
    // outFile << json::dump(jointAngles, outFile) << ;
    // outFile.close();
}

void ServoMotorController::load_joint_angles() {
    //  std::ifstream inFile("oint_angles.json");
    // if (inFile.is_open()) {
    //     Json data = Json::parse(inFile);
    // }else{
    //     Logger::error("Error opening file: example.json");
    //     return;
    // }
    //
    // inFile.close();
}

void ServoMotorController::set_current_joint_angles(std::map<ServoMotorJoint, uint8_t>& angles) {
    for (const auto& [joint, angle] : angles) {
        Logger::info("{} : {}", Servo_Motor_Joint_Names.at(joint) , std::to_string(angle));

        set_absolute_servo_angle(joint, angle);
    }

    save_joint_angles();
}

void ServoMotorController::set_absolute_servo_angle(ServoMotorJoint joint, int targetAngle, int step, int delayMs) {
    int currentAngle = _currentJointAngles[joint];
    int direction = (targetAngle > currentAngle) ? 1 : -1;

    while (currentAngle != targetAngle) {
        currentAngle += direction * step;

        if ((direction == 1 && currentAngle > targetAngle) ||
            (direction == -1 && currentAngle < targetAngle)) {
            currentAngle = targetAngle;
        }

        int pulse = 500 + (currentAngle / 180.0) * 2000;
        _driver.set_servo_pulse(_currentJointAngles[joint], pulse);
        usleep(delayMs * 1000);
    }
    _currentJointAngles[joint] = targetAngle;
}

std::map<ServoMotorJoint, uint8_t> ServoMotorController::get_current_joint_angles() {
    return _currentJointAngles;
}

