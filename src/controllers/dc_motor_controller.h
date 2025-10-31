#ifndef DC_MOTOR_CONTROLLER_H
#define DC_MOTOR_CONTROLLER_H

#include <atomic>
#include "controller_data_structures.h"
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

    DCMotorState getCurrentState(){return _currentState;};

    void forward(int magnitude);
    void backward(int magnitude);
    void turnRight(int magnitude);
    void turnLeft(int magnitude);
    void stop();
    void setEnable(bool enable){ _enable.store(enable);}

private:
    DCMotorController(int address);
    ~DCMotorController();

    void run(DCMotor motor, DCMotorDirection direction, int speed);
    void setDirection(int pin1, int pin2, DCMotorDirection direction);

    static DCMotorController* _instance;
    PCA9685Driver _driver;
    DCMotorState _currentState;
    std::atomic<bool> _enable = true;
    std::atomic<bool> _initialized = false;
};

#endif // MOTOR_DRIVER_H