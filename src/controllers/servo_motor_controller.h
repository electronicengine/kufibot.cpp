#ifndef SERVO_MOTOR_CONTROLLER_H
#define SERVO_MOTOR_CONTROLLER_H

#include <map>
#include <memory>

#include "../drivers/pca9685_driver.h"
#include "../gesture_defs.h"
#include "controller.h"
#include "controller_data_structures.h"

class ServoMotorController : public Controller {
public:
    static ServoMotorController *get_instance(int address = 0x40);
    ~ServoMotorController() override;

    bool initialize() override;
    void shutdown() override;
    bool isReady() const noexcept override;

    void setJointAngle(ServoMotorJoint joint, int targetAngle, int step = 1, int delayMs = 5);
    std::map<ServoMotorJoint, uint8_t> getAllJointsAngle();
    void setAllJointsAngle(std::map<ServoMotorJoint, uint8_t>& angles);


private:
    ServoMotorController(int address);
    int _address;
    const std::string SAVE_PATH ="/home/kufi/.config/kufi/servo_joint_angles.json";

    static ServoMotorController* _instance;
    PCA9685Driver _driver;
    std::map<ServoMotorJoint, uint8_t> _currentJointAngles;
    std::unique_ptr<std::ofstream> _saveFile;
    void saveJointAngles();
    void loadJointAngles();
    void initSaveFile();
};

#endif