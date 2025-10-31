/*
* This file is part of Kufibot.
 *
 * Kufibot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kufibot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kufibot. If not, see <https://www.gnu.org/licenses/>.
 */

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
    bool ret = _driver.initPCA9685(address);
    if (ret) {
        _driver.setPWMFrequency(50);
        _currentJointAngles = {
            {ServoMotorJoint::rightArm, 15}, {ServoMotorJoint::leftArm, 170}, {ServoMotorJoint::neck, 78},
            {ServoMotorJoint::headUpDown, 15}, {ServoMotorJoint::headLeftRight, 90}, {ServoMotorJoint::eyeRight, 160},
            {ServoMotorJoint::eyeLeft, 20}
        };
        _initialized = true;

        INFO("Servo driver initialized");
    }else {
        _initialized = false;

        ERROR("Servo driver initialization failed");
    }
}


void ServoMotorController::saveJointAngles()
{
    // std::ofstream outFile("joint_angles.json");
    // outFile << json::dump(jointAngles, outFile) << ;
    // outFile.close();
}

void ServoMotorController::loadJointAngles() {
    //  std::ifstream inFile("oint_angles.json");
    // if (inFile.is_open()) {
    //     Json data = Json::parse(inFile);
    // }else{
    //     ERROR("Error opening file: example.json");
    //     return;
    // }
    //
    // inFile.close();
}

void ServoMotorController::setAllJointsAngle(std::map<ServoMotorJoint, uint8_t>& angles) {
    for (const auto& [joint, angle] : angles) {
        INFO("{} : {}", Servo_Motor_Joint_Names.at(joint) , std::to_string(angle));

        setJointAngle(joint, angle);
    }

    saveJointAngles();
}

void ServoMotorController::setJointAngle(ServoMotorJoint joint, int targetAngle, int step, int delayMs) {
    if (!_enable.load() || !_initialized) {
        return;
    }

    int currentAngle = _currentJointAngles[joint];
    int direction = (targetAngle > currentAngle) ? 1 : -1;

    while (currentAngle != targetAngle) {
        currentAngle += direction * step;

        if ((direction == 1 && currentAngle > targetAngle) ||
            (direction == -1 && currentAngle < targetAngle)) {
            currentAngle = targetAngle;
        }

        // controlling Servo 500–2500 μs pulse interval.
        // 500 μs → 500 / 20000 * 100 = 2.5%
        // 2500 μs → 2500 / 20000 * 100 = 12.5%
        int pulse = 500 + (currentAngle / 180.0) * 2000;
        _driver.setDutyCyclePulse((int)joint, pulse);
        usleep(delayMs * 1000);
    }
    _currentJointAngles[joint] = targetAngle;
}

std::map<ServoMotorJoint, uint8_t> ServoMotorController::getAllJointsAngle() {
    return _currentJointAngles;
}

