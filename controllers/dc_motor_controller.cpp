#include "dc_motor_controller.h"

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

    // Define direction strings (optional)
    _directions["forward"] = 0;
    _directions["backward"] = 1;

    // Define motor mapping (optional)
    _motor["right"] = 0;
    _motor["left"] = 1;
}

DCMotorController::~DCMotorController() {
    stop(); // Ensure motors stop on object destruction
}


void DCMotorController::run(int motor, const std::string& direction, int speed) {
    if (speed > 100) {
        std::cout << "Speed must be between 0 and 100." << std::endl;
    }

    if(motor == 0){
        _driver.set_duty_cycle(PWMA, speed);
        set_direction(AIN1, AIN2, direction);
    }else{
        _driver.set_duty_cycle(PWMB, speed);
        set_direction(BIN1, BIN2, direction);
    }

}

void DCMotorController::set_direction(int pin1, int pin2, const std::string& direction) {

    if(direction == "forward"){
        _driver.set_level(pin1, 0);
        _driver.set_level(pin2, 1);
    }else if(direction == "backward"){
        _driver.set_level(pin1, 1);
        _driver.set_level(pin2, 0);
    }
}

void DCMotorController::forward(int magnitude) {
    run(_motor["right"], "forward", magnitude);
    run(_motor["left"],  "forward", magnitude);
}

void DCMotorController::backward(int magnitude) {
    run(_motor["right"],  "backward", magnitude);
    run(_motor["left"],  "backward", magnitude);
}

void DCMotorController::turn_right(int magnitude) {
    run(_motor["right"],  "forward", magnitude);
    run(_motor["left"], "backward", magnitude);
}

void DCMotorController::turn_left(int magnitude) {
    run(_motor["right"], "backward", magnitude);
    run(_motor["left"], "forward", magnitude);
}

void DCMotorController::stop() {
    _driver.set_duty_cycle(PWMA, 0);
    _driver.set_duty_cycle(PWMB, 0);
}

