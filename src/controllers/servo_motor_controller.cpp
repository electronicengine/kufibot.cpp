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

    _jointChannels = {
        {"right_arm", 0}, {"left_arm", 1}, {"neck_down", 2},
        {"neck_up", 3}, {"neck_right", 4}, {"eye_right", 5},
        {"eye_left", 6}
    };

    _jointAngles = {
        {"right_arm", 15}, {"left_arm", 170}, {"neck_down", 78},
        {"neck_up", 15}, {"neck_right", 90}, {"eye_right", 160},
        {"eye_left", 20}
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

     std::ifstream inFile("oint_angles.json");
    if (inFile.is_open()) {
        Json data = Json::parse(inFile);
    }else{
        Logger::error("Error opening file: example.json");
        return;
    }

    inFile.close();

}

void ServoMotorController::set_all_angles(const std::map<std::string, int>& angles) {
    for (const auto& [joint, angle] : angles) {
        Logger::info("{} : {}", joint , std::to_string(angle));

        set_absolute_servo_angle(joint, angle);
    }

    save_joint_angles();
}

void ServoMotorController::set_absolute_servo_angle(const std::string& joint, int targetAngle, int step, int delayMs) {
    int currentAngle = _jointAngles[joint];
    int direction = (targetAngle > currentAngle) ? 1 : -1;

    while (currentAngle != targetAngle) {
        currentAngle += direction * step;

        if ((direction == 1 && currentAngle > targetAngle) ||
            (direction == -1 && currentAngle < targetAngle)) {
            currentAngle = targetAngle;
        }

        int pulse = 500 + (currentAngle / 180.0) * 2000;
        _driver.set_servo_pulse(_jointChannels[joint], pulse);
        usleep(delayMs * 1000);
    }

    _jointAngles[joint] = targetAngle;
}

const std::map<std::string, int>& ServoMotorController::get_all_joint_angles() {
    return _jointAngles;
}

std::map<std::string, int> ServoMotorController::get_joint_channels() {
    return _jointChannels;
}

void ServoMotorController::head_down() {
    int targetAngle = _jointAngles["neck_up"] + 1;
    if (targetAngle >= 180) {
        targetAngle = 180;
    }
    set_absolute_servo_angle("neck_up", targetAngle);
    _jointAngles["neck_up"] = targetAngle;

    targetAngle = _jointAngles["neck_down"] - 1;
    if (targetAngle <= 0) {
        targetAngle = 0;
    }
    set_absolute_servo_angle("neck_down", targetAngle);
    _jointAngles["neck_down"] = targetAngle;
    save_joint_angles();
}

void ServoMotorController::head_up() {
    int targetAngle = _jointAngles["neck_up"] - 1;
    if (targetAngle <= 20) {
        targetAngle = 20;
    }
    set_absolute_servo_angle("neck_up", targetAngle);
    _jointAngles["neck_up"] = targetAngle;

    targetAngle = _jointAngles["neck_down"] + 1;
    if (targetAngle >= 180) {
        targetAngle = 180;
    }
    set_absolute_servo_angle("neck_down", targetAngle);
    _jointAngles["neck_down"] = targetAngle;
    save_joint_angles();
}

void ServoMotorController::head_left() {
    int targetAngle = _jointAngles["neck_right"] + 1;
    if (targetAngle >= 180) {
        targetAngle = 180;
    }
    set_absolute_servo_angle("neck_right", targetAngle);
    _jointAngles["neck_right"] = targetAngle;
    save_joint_angles();
}

void ServoMotorController::head_right() {
    int targetAngle = _jointAngles["neck_right"] - 1;
    if (targetAngle <= 0) {
        targetAngle = 0;
    }
    set_absolute_servo_angle("neck_right", targetAngle);
    _jointAngles["neck_right"] = targetAngle;
    save_joint_angles();
}

void ServoMotorController::eye_up() {
    set_absolute_servo_angle("eye_right", 170);
    set_absolute_servo_angle("eye_left", 20);
    save_joint_angles();
}

void ServoMotorController::eye_down() {
    set_absolute_servo_angle("eye_right", 130);
    set_absolute_servo_angle("eye_left", 50);
    save_joint_angles();
}

void ServoMotorController::eye_angry() {
    set_absolute_servo_angle("eye_right", 180);
    set_absolute_servo_angle("eye_left", 0);
    save_joint_angles();
}

void ServoMotorController::eye_wondering()
{
    set_absolute_servo_angle("eye_right", 130);
    set_absolute_servo_angle("eye_left", 0);
    save_joint_angles();
}
