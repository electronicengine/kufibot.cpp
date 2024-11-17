#ifndef DC_MOTOR_CONTROLLER_H
#define DC_MOTOR_CONTROLLER_H

#include <iostream>
#include <wiringPi.h>
#include <map>
#include <string>

#include "../drivers/pca9685_driver.h"

// Define motor pin mappings
#define PWMA 0
#define AIN1 1
#define AIN2 2
#define PWMB 5
#define BIN1 3
#define BIN2 4


class DCMotorController {
public:
    static DCMotorController* get_instance(int address = 0x50);

    DCMotorController(const DCMotorController&) = delete;
    DCMotorController& operator=(const DCMotorController&) = delete;

    void forward(int magnitude);
    void backward(int magnitude);
    void turn_right(int magnitude);
    void turn_left(int magnitude);
    void stop();

private:
    DCMotorController(int address);
    ~DCMotorController();

    void run(int motor, const std::string& direction, int speed);
    void set_direction(int pin1, int pin2, const std::string& direction);

    static DCMotorController* _instance;
    PCA9685Driver _driver;
    std::map<std::string, int> _directions;
    std::map<std::string, int> _motor;
};

#endif // MOTOR_DRIVER_H