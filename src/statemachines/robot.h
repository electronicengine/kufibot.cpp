//
// Created by ybulb on 8/9/2025.
//

#ifndef ROBOT_H
#define ROBOT_H

#include "../controllers/compass_controller.h"
#include "../controllers/dc_motor_controller.h"
#include "../controllers/distance_controller.h"
#include "../controllers/power_controller.h"
#include "../controllers/servo_motor_controller.h"
#include "critical_error_state.h"
#include "idle_state.h"
#include "initialize_state.h"
#include "moving_state.h"
#include "remote_control_state.h"
#include "state.h"
#include "state_machine.h"
#include "talking_state.h"
#include "tracking_state.h"
#include "tui_control_state.h"


struct Robot : StateMachine {

    IdleState* idleState = nullptr;
    MovingState* movingState = nullptr;
    TuiControlState* tuiControlState = nullptr;
    RemoteControlState* remoteControlState = nullptr;
    InitializeState* initializeState = nullptr;
    CriticalErrorState* criticalErrorState = nullptr;
    TalkingState* talkingState = nullptr;
    TrackingState* trackingState = nullptr;


    CompassController *_compassController;
    DistanceController *_distanceController;
    PowerController *_powerController;
    ServoMotorController *_servoController;
    DCMotorController *_dcMotorController;

    std::mutex _controllerMutex;
    std::atomic<bool> _enableSensorContinuousReadings = false;
    std::map<ServoMotorJoint, std::map<GestureJointState, GestureJointAngle>> _jointLimits;

    Robot();
    void dumpActivePath();
    bool initialize();
    void setEnableSensorContinuousReadings(bool enable);
    SensorData getCurrentMotorPositions();
    SensorData get_sensor_values();
    void control_motion(const ControlData& controlData);
    void control_body(int angle, int magnitude);
    void control_head(int angle, int magnitude);
    void control_arm(ServoMotorJoint joint, int angle, bool scale = true) const;
    void control_eye(ServoMotorJoint joint, bool state);

    void head_down(int speed);
    void head_up(int speed);
    void head_left(int speed);
    void head_right(int speed);
    void eye_up();
    void eye_down();

};


#endif //ROBOT_H
