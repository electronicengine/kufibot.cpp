//
// Created by ybulb on 8/9/2025.
//

#ifndef ROBOT_H
#define ROBOT_H

#include "state_machine.h"
#include "state.h"
#include "idle_state.h"
#include "moving_state.h"
#include "tui_control_state.h"
#include "remote_control_state.h"
#include "initialize_state.h"
#include "critical_error_state.h"
#include "talking_state.h"
#include "../controllers/compass_controller.h"
#include "../controllers/distance_controller.h"
#include "../controllers/power_controller.h"
#include "../controllers/servo_motor_controller.h"
#include "../controllers/dc_motor_controller.h"


struct Robot : StateMachine {

    IdleState* idleState = nullptr;
    MovingState* movingState = nullptr;
    TuiControlState* tuiControlState = nullptr;
    RemoteControlState* remoteControlState = nullptr;
    InitializeState* initializeState = nullptr;
    CriticalErrorState* criticalErrorState = nullptr;
    TalkingState* talkingState = nullptr;

    CompassController *_compassController;
    DistanceController *_distanceController;
    PowerController *_powerController;
    ServoMotorController *_servoController;
    DCMotorController *_dcMotorController;

    std::mutex _controllerMutex;

    Robot();
    void dumpActivePath();
    bool initialize();
    SensorData get_sensor_values();
    void control_motion(const ControlData& controlData);
    void control_body(int angle, int magnitude);
    void control_head(int angle, int magnitude);
    void control_arm(ServoMotorJoint joint, int angle, bool scale = true) const;
    void control_eye(ServoMotorJoint joint, bool state);

    void head_down();
    void head_up();
    void head_left();
    void head_right();
    void eye_up();
    void eye_down();

};


#endif //ROBOT_H
