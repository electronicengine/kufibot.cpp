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

ServoMotorController::ServoMotorController(int address)
    : Controller("ServoMotorController"), _address(address) {
    if (!ServoMotorController::initialize()) {
        WARNING("{} failed to initialize", getName());
    }
}

bool ServoMotorController::initialize() {
    bool ret = _driver.initPCA9685(_address);
    if (ret) {
        _driver.setPWMFrequency(50);
        _currentJointAngles = {
            {ServoMotorJoint::rightArm, 15}, {ServoMotorJoint::leftArm, 170}, {ServoMotorJoint::neck, 30},
            {ServoMotorJoint::headUpDown, 15}, {ServoMotorJoint::headLeftRight, 90}, {ServoMotorJoint::eyeRight, 160},
            {ServoMotorJoint::eyeLeft, 20}
        };
        _initialized.store(true);
        initSaveFile();
        loadJointAngles();
        INFO("Servo driver initialized");
    }else {
        _initialized.store(false);

        ERROR("Servo driver initialization failed");
    }

    return ret;
}


void ServoMotorController::shutdown() {
    _driver.shutdown();
    _initialized.store(false);
}

bool ServoMotorController::isReady() const noexcept {
    return _initialized.load();
}

ServoMotorController::~ServoMotorController() {
    ServoMotorController::shutdown();
}

void ServoMotorController::initSaveFile() {
    // Dosya yoksa oluştur, varsa dokunma
    std::ofstream createFile(SAVE_PATH, std::ios::app);
    if (!createFile.is_open()) {
        ERROR("Failed to create joint_angles.json");
        return;
    }
    createFile.close();
}

void ServoMotorController::saveJointAngles() {
    std::ofstream outFile(SAVE_PATH, std::ios::out | std::ios::trunc);
    if (!outFile.is_open()) {
        ERROR("Failed to open joint_angles.json for writing");
        return;
    }

    Json data;
    for (const auto& [joint, angle] : _currentJointAngles) {
        data[Servo_Motor_Joint_Names.at(joint)] = angle;
    }
    outFile << data.dump(4);
    outFile.close();
}

void ServoMotorController::loadJointAngles() {

    std::ifstream inFile(SAVE_PATH);
    if (!inFile.is_open()) {
        ERROR("Error opening file: servo_joint_angles.json");
        return;
    }

    try {
        std::stringstream buffer;
        buffer << inFile.rdbuf();
        inFile.close();

        std::string content = buffer.str();
        if (content.empty()) {
            // Dosya boşsa yüklenecek bir şey yok, hata değil
            return;
        }

        Json data = Json::parse(content, nullptr, false);
        if (data.is_discarded()) {
            ERROR("Invalid JSON format in file: servo_joint_angles.json");
            return;
        }

        for (auto& [joint, angle] : _currentJointAngles) {
            angle = data.value(Servo_Motor_Joint_Names.at(joint), angle);
        }
    } catch (const std::exception& e) {
        ERROR("Failed to parse JSON: {}", e.what());
    }
}



void ServoMotorController::setAllJointsAngle(std::map<ServoMotorJoint, uint8_t>& angles) {
    for (const auto& [joint, angle] : angles) {
        INFO("{} : {}", Servo_Motor_Joint_Names.at(joint) , std::to_string(angle));

        setJointAngle(joint, angle);
    }

}

void ServoMotorController::setJointAngle(ServoMotorJoint joint, int targetAngle, int step, int delayMs) {
    if (!isEnabled() || !isReady()) {
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
        _currentJointAngles[joint] = currentAngle;
        saveJointAngles();
    }
    _currentJointAngles[joint] = targetAngle;
}

std::map<ServoMotorJoint, uint8_t> ServoMotorController::getAllJointsAngle() {
    return _currentJointAngles;
}

