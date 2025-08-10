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

DCMotorController::DCMotorController(int address = 0x50) {
    _driver = PCA9685Driver(address); // Initialize I2C communication
    _driver.set_pwm_freq(50); // Set PWM frequency

}

DCMotorController::~DCMotorController() {
    stop(); // Ensure motors stop on object destruction
}


void DCMotorController::run(DCMotor motor, DCMotorDirection direction, int speed) {
    if (speed > 100) {
        WARNING("Speed must be between 0 and 100.");
    }

    if(motor == DCMotor::left){
        if (direction == DCMotorDirection::forward)
            direction = DCMotorDirection::backward;
        else
            direction = DCMotorDirection::forward;

        _driver.set_duty_cycle(PWMA, speed);
        set_direction(AIN1, AIN2, direction);
    }else{
        _driver.set_duty_cycle(PWMB, speed);
        set_direction(BIN1, BIN2, direction);
    }

}

void DCMotorController::set_direction(int pin1, int pin2, DCMotorDirection direction) {

    if(direction == DCMotorDirection::forward){
        _driver.set_level(pin1, 0);
        _driver.set_level(pin2, 1);
    }else if(direction == DCMotorDirection::backward){
        _driver.set_level(pin1, 1);
        _driver.set_level(pin2, 0);
    }
}

void DCMotorController::forward(int magnitude) {
    run(DCMotor::right, DCMotorDirection::forward, magnitude);
    run(DCMotor::left,  DCMotorDirection::forward, magnitude);
}

void DCMotorController::backward(int magnitude) {
    run(DCMotor::right,  DCMotorDirection::backward, magnitude);
    run(DCMotor::left,  DCMotorDirection::backward, magnitude);
}

void DCMotorController::turn_right(int magnitude) {
    run(DCMotor::right,  DCMotorDirection::forward, magnitude);
    run(DCMotor::left, DCMotorDirection::backward, magnitude);
}

void DCMotorController::turn_left(int magnitude) {
    run(DCMotor::right, DCMotorDirection::backward, magnitude);
    run(DCMotor::left, DCMotorDirection::forward, magnitude);
}

void DCMotorController::stop() {
    _driver.set_duty_cycle(PWMA, 0);
    _driver.set_duty_cycle(PWMB, 0);
}

