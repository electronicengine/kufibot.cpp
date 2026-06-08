#ifndef SERVO_MOTOR_CONTROLLER_H
#define SERVO_MOTOR_CONTROLLER_H

#include <map>
#include "controller.h"
#include "controller_data_structures.h"
#include "../drivers/pca9685_driver.h"
#include "../gesture_defs.h"

class ServoMotorController : public Controller {
public:
    static ServoMotorController *get_instance(int address = 0x40);
    ~ServoMotorController() override;

    bool initialize() override;
    void shutdown() override;
    bool isReady() const noexcept override;

    void setJointAngle(ServoMotorJoint joint, int targetAngle, int step = 1, int delayMs = 4);
    std::map<ServoMotorJoint, uint8_t> getAllJointsAngle();
    void setAllJointsAngle(std::map<ServoMotorJoint, uint8_t>& angles);


private:
    ServoMotorController(int address);
    int _address;

    static ServoMotorController* _instance;
    PCA9685Driver _driver;
    std::map<ServoMotorJoint, uint8_t> _currentJointAngles;

    void saveJointAngles();
    void loadJointAngles();
};

#endif