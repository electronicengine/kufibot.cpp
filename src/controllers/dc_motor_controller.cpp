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

#include "dc_motor_controller.h"
#include "../logger.h"

DCMotorController* DCMotorController::_instance = nullptr;

DCMotorController* DCMotorController::get_instance(int address) {
    if (_instance == nullptr) {
        _instance = new DCMotorController(address);
    }
    return _instance;
}

DCMotorController::DCMotorController(int address)
    : Controller("DCMotorController"), _address(address) {
    if (!DCMotorController::initialize()) {
        WARNING("{} failed to initialize", getName());
    }
}

bool DCMotorController::initialize() {
    bool ret = _driver.initPCA9685(_address);
    if (ret) {
        _driver.setPWMFrequency(50); // Set PWM frequency
        _initialized.store(true);
        INFO("DCMotorController is initialized!");
    }else {
        _initialized.store(false);
        ERROR("DCMotorController initialization failed!");
    }

    return ret;
}

void DCMotorController::shutdown() {
    if (_initialized.load()) {
        stop();
    }
    _driver.shutdown();
    _initialized.store(false);
}

bool DCMotorController::isReady() const noexcept {
    return _initialized.load();
}

DCMotorController::~DCMotorController() {
    DCMotorController::shutdown(); // Ensure motors stop on object destruction
}


void DCMotorController::run(DCMotor motor, DCMotorDirection direction, int speed) {
    if (!isEnabled() || !isReady())
        return;

    if (speed > 100) {
        WARNING("Speed must be between 0 and 100.");
    }

    if(motor == DCMotor::left){
        if (direction == DCMotorDirection::forward)
            direction = DCMotorDirection::backward;
        else
            direction = DCMotorDirection::forward;

        _driver.setDutyCyclePercent(PWMA, speed);
        setDirection(AIN1, AIN2, direction);
    }else{
        _driver.setDutyCyclePercent(PWMB, speed);
        setDirection(BIN1, BIN2, direction);
    }

}

void DCMotorController::setDirection(int pin1, int pin2, DCMotorDirection direction) {

    if(direction == DCMotorDirection::forward){
        _driver.setChannelLevel(pin1, 0);
        _driver.setChannelLevel(pin2, 1);
    }else if(direction == DCMotorDirection::backward){
        _driver.setChannelLevel(pin1, 1);
        _driver.setChannelLevel(pin2, 0);
    }
}

void DCMotorController::forward(int magnitude) {
    if (!isEnabled() || !isReady())
        return;

    run(DCMotor::right, DCMotorDirection::forward, magnitude);
    run(DCMotor::left,  DCMotorDirection::forward, magnitude);
}

void DCMotorController::backward(int magnitude) {
    if (!isEnabled() || !isReady())
        return;

    run(DCMotor::right,  DCMotorDirection::backward, magnitude);
    run(DCMotor::left,  DCMotorDirection::backward, magnitude);
}

void DCMotorController::turnRight(int magnitude) {
    if (!isEnabled() || !isReady())
        return;

    run(DCMotor::right,  DCMotorDirection::forward, magnitude);
    run(DCMotor::left, DCMotorDirection::backward, magnitude);
}

void DCMotorController::turnLeft(int magnitude) {
    if (!isEnabled() || !isReady())
        return;

    run(DCMotor::right, DCMotorDirection::backward, magnitude);
    run(DCMotor::left, DCMotorDirection::forward, magnitude);
}

void DCMotorController::stop() {
    if (!isEnabled() || !isReady())
        return;
    
    _driver.setDutyCyclePercent(PWMA, 0);
    _driver.setDutyCyclePercent(PWMB, 0);
}

