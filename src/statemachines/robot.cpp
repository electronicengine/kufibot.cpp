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

#include "robot.h"
#include "../logger.h"

// Robot
Robot::Robot() {
    idleState = createState<IdleState>("IdleState");
    movingState = createState<MovingState>("MovingState");
    criticalErrorState = createState<CriticalErrorState>("CriticalErrorState");
    initializeState = createState<InitializeState>("InitializeState");

    tuiControlState = createState<TuiControlState>("TuiControlState", movingState);
    remoteControlState = createState<RemoteControlState>("RemoteControlState", movingState);
    talkingState = createState<TalkingState>("TalkingState", movingState);

    movingState->setTimeout(5000);
    setInitialState(initializeState);
}



void Robot::dumpActivePath() {
    std::vector<std::string> path;
    State* s = _currentState;
    while (s) { path.push_back(s->_name); s = s->_parentState; }
    std::cout << "Active path: ";
    for (size_t i = 0; i < path.size(); i++) {
        std::cout << path[i];
        if (i + 1 < path.size()) std::cout << " -> ";
    }
    std::cout << "\n";
}

bool Robot::initialize() {

    _compassController =  CompassController::get_instance();
    _distanceController = DistanceController::get_instance();
    _powerController = PowerController::get_instance();
    _servoController = ServoMotorController::get_instance();
    _dcMotorController = DCMotorController::get_instance();

    _compassController->setEnable(false);
    _distanceController->setEnable(false);
    _powerController->setEnable(false);
    _servoController->setEnable(false);
    _dcMotorController->setEnable(false);

    return true;
}

void Robot::setEnableSensorContinuousReadings(bool enable) {

    _compassController->setEnable(enable);
    _distanceController->setEnable(enable);
    _powerController->setEnable(enable);
    _servoController->setEnable(!enable);
    _dcMotorController->setEnable(!enable);

    _enableSensorContinuousReadings = enable;
}

SensorData Robot::get_sensor_values()
{
    std::lock_guard<std::mutex> lock(_controllerMutex);

    SensorData sensorData;

    if (_enableSensorContinuousReadings) {
        _compassController->setEnable(true);
        _distanceController->setEnable(true);
        _powerController->setEnable(true);

        sensorData.compassData = _compassController->get_all();
        sensorData.distanceData = _distanceController->get_distance();
        sensorData.powerData = _powerController->get_consumption();
        sensorData.currentJointAngles = _servoController->get_current_joint_angles();
        sensorData.dcMotorState = _dcMotorController->get_current_state();

        return sensorData;
    }else {
        _compassController->setEnable(false);
        _distanceController->setEnable(false);
        _powerController->setEnable(false);

        return sensorData;
    }

}

void Robot::control_motion(const ControlData& controlData)
{
    std::lock_guard<std::mutex> lock(_controllerMutex);

    if (controlData.bodyJoystick.has_value()) {
        control_body(controlData.bodyJoystick->angle, controlData.bodyJoystick->strength);
    }else if (controlData.headJoystick.has_value()) {
        control_head(controlData.headJoystick->angle, controlData.headJoystick->strength);
    }else if (controlData.leftArmAngle.has_value()) {
        control_arm(ServoMotorJoint::leftArm, controlData.leftArmAngle.value());
    }else if (controlData.rightArmAngle.has_value()) {
        control_arm(ServoMotorJoint::rightArm, controlData.rightArmAngle.value());
    }else if (controlData.leftEye.has_value()) {
        control_eye(ServoMotorJoint::eyeLeft, controlData.leftEye.value());
    }else if (controlData.rightEye.has_value()) {
        control_eye(ServoMotorJoint::rightArm, controlData.rightEye.value());
    }else if (controlData.jointAngles.has_value()) {
        std::map<ServoMotorJoint, uint8_t> jointAngles = _servoController->get_current_joint_angles();

        if (jointAngles.at(ServoMotorJoint::leftArm) != controlData.jointAngles->at(ServoMotorJoint::leftArm)) {
            INFO("setting leftArm absolute servo angle: {}", controlData.jointAngles->at(ServoMotorJoint::leftArm));
            _servoController->set_absolute_servo_angle(ServoMotorJoint::leftArm, controlData.jointAngles->at(ServoMotorJoint::leftArm));
        }
        if (jointAngles.at(ServoMotorJoint::rightArm) != controlData.jointAngles->at(ServoMotorJoint::rightArm)) {
            INFO("setting rightArm absolute servo angle: {}", controlData.jointAngles->at(ServoMotorJoint::rightArm));
            _servoController->set_absolute_servo_angle(ServoMotorJoint::rightArm, controlData.jointAngles->at(ServoMotorJoint::rightArm));
        }
        if (jointAngles.at(ServoMotorJoint::neck) != controlData.jointAngles->at(ServoMotorJoint::neck)) {
            INFO("setting neck absolute servo angle: {}", controlData.jointAngles->at(ServoMotorJoint::neck));
            _servoController->set_absolute_servo_angle(ServoMotorJoint::neck, controlData.jointAngles->at(ServoMotorJoint::neck));
        }
        if (jointAngles.at(ServoMotorJoint::headUpDown) != controlData.jointAngles->at(ServoMotorJoint::headUpDown)) {
            INFO("setting headUpDown absolute servo angle: {}", controlData.jointAngles->at(ServoMotorJoint::headUpDown));
            _servoController->set_absolute_servo_angle(ServoMotorJoint::headUpDown, controlData.jointAngles->at(ServoMotorJoint::headUpDown));
        }
        if (jointAngles.at(ServoMotorJoint::headLeftRight) != controlData.jointAngles->at(ServoMotorJoint::headLeftRight)) {
            INFO("setting headLeftRight absolute servo angle: {}", controlData.jointAngles->at(ServoMotorJoint::headLeftRight));
            _servoController->set_absolute_servo_angle(ServoMotorJoint::headLeftRight, controlData.jointAngles->at(ServoMotorJoint::headLeftRight));
        }
        if (jointAngles.at(ServoMotorJoint::eyeLeft) != controlData.jointAngles->at(ServoMotorJoint::eyeLeft)) {
            INFO("setting eyeLeft absolute servo angle: {}", controlData.jointAngles->at(ServoMotorJoint::eyeLeft));
            _servoController->set_absolute_servo_angle(ServoMotorJoint::eyeLeft, controlData.jointAngles->at(ServoMotorJoint::eyeLeft));
        }
        if (jointAngles.at(ServoMotorJoint::eyeRight) != controlData.jointAngles->at(ServoMotorJoint::eyeRight)) {
            INFO("setting eyeRight absolute servo angle: {}", controlData.jointAngles->at(ServoMotorJoint::eyeRight));
            _servoController->set_absolute_servo_angle(ServoMotorJoint::eyeRight, controlData.jointAngles->at(ServoMotorJoint::eyeRight));
        }
    }
}

void Robot::control_body(int angle, int magnitude) {

    if (!_dcMotorController) {
        return;
    }

    if (magnitude == 100) {
        magnitude = 95;
    }

    INFO("control_body: {} - magnitude: {}", std::to_string(angle), std::to_string(magnitude));

    if (magnitude == 0 && angle == 0) {
        _dcMotorController->stop();
        return;
    }

    // Normalize angle to [-180, 180]
    angle = ((angle + 180) % 360) - 180;

    if (angle >= UP_MIN && angle < UP_MAX) {
        _dcMotorController->forward(magnitude);
    } else if (angle >= RIGHT_MIN && angle < RIGHT_MAX) {
        _dcMotorController->turn_right(magnitude);
    } else if (angle >= DOWN_MIN && angle < DOWN_MAX) {
        _dcMotorController->backward(magnitude);
    } else if (angle >= LEFT_MIN || angle < LEFT_MAX) {
        _dcMotorController->turn_left(magnitude);
    } else if (angle == 0 || magnitude == 0) {
        _dcMotorController->stop();  // Optional: fallback safety
    }else {
        WARNING("control_body: unknown angle: {} - magnitude: {}", std::to_string(angle), std::to_string(magnitude));
    }
}

void Robot::control_head(int angle, int magnitude) {
    INFO("Control head: {} - magnitude: {}", std::to_string(angle), std::to_string(magnitude));
    if (!_servoController || (magnitude == 0 && angle == 0)) {
        return;
    }
    // Normalize angle to range [-180, 180]
    angle = ((angle + 180) % 360) - 180;

    if (angle >= UP_MIN && angle < UP_MAX) {
        head_up();
    } else if (angle >= RIGHT_MIN && angle < RIGHT_MAX) {
        head_right();
    } else if (angle >= DOWN_MIN && angle < DOWN_MAX) {
        head_down();
    } else {
        head_left(); // Covers angle >= 135 or angle < -135
    }
}

void Robot::control_arm(ServoMotorJoint joint, int angle, bool scale) const {
    INFO("control_arm : {} ", angle);

    if(_servoController){
        double mapped_angle;
        if(scale){
            mapped_angle = (angle / 100.0) * 180.0;
        }else {
            mapped_angle = angle;
        }

        INFO("control_arm: {}", Servo_Motor_Joint_Names.at(joint));
        _servoController->set_absolute_servo_angle(joint, mapped_angle);
    }
}


void Robot::control_eye(ServoMotorJoint joint, bool state) {
    INFO("control_eye : {}", state);

    if(_servoController){
        if (joint == ServoMotorJoint::eyeLeft) {
            if (state)
                _servoController->set_absolute_servo_angle(ServoMotorJoint::eyeLeft, 20);
            else
                _servoController->set_absolute_servo_angle(ServoMotorJoint::eyeLeft, 50);

            //eyes Up
        } else if (joint == ServoMotorJoint::eyeRight) {
            if (state)
                _servoController->set_absolute_servo_angle(ServoMotorJoint::eyeRight, 170);
            else
                _servoController->set_absolute_servo_angle(ServoMotorJoint::eyeRight, 130);
        }
    }

}


void Robot::head_down() {
    INFO("Head Down");

    std::map<ServoMotorJoint, uint8_t> currentJointAngles = _servoController->get_current_joint_angles();
    int targetAngle = currentJointAngles[ServoMotorJoint::headUpDown] + 1;
    if (targetAngle >= 180) {
        targetAngle = 180;
    }
    _servoController->set_absolute_servo_angle(ServoMotorJoint::headUpDown, targetAngle);
    currentJointAngles[ServoMotorJoint::headUpDown] = targetAngle;

    targetAngle = currentJointAngles[ServoMotorJoint::neck] - 1;
    if (targetAngle <= 0) {
        targetAngle = 0;
    }
    _servoController->set_absolute_servo_angle(ServoMotorJoint::neck, targetAngle);
    currentJointAngles[ServoMotorJoint::neck] = targetAngle;
}

void Robot::head_up() {
    INFO("Head Up");
    std::map<ServoMotorJoint, uint8_t> currentJointAngles = _servoController->get_current_joint_angles();

    int targetAngle = currentJointAngles[ServoMotorJoint::headUpDown] - 1;
    if (targetAngle <= 20) {
        targetAngle = 20;
    }
    _servoController->set_absolute_servo_angle(ServoMotorJoint::headUpDown, targetAngle);
    currentJointAngles[ServoMotorJoint::headUpDown] = targetAngle;

    targetAngle = currentJointAngles[ServoMotorJoint::neck] + 1;
    if (targetAngle >= 180) {
        targetAngle = 180;
    }
    _servoController->set_absolute_servo_angle(ServoMotorJoint::neck, targetAngle);
    currentJointAngles[ServoMotorJoint::neck] = targetAngle;
}

void Robot::head_left() {
    INFO("Head Left");

    std::map<ServoMotorJoint, uint8_t> currentJointAngles = _servoController->get_current_joint_angles();

    int targetAngle = currentJointAngles[ServoMotorJoint::headLeftRight] + 1;
    if (targetAngle >= 180) {
        targetAngle = 180;
    }
    _servoController->set_absolute_servo_angle(ServoMotorJoint::headLeftRight, targetAngle);
    currentJointAngles[ServoMotorJoint::headLeftRight] = targetAngle;
}

void Robot::head_right() {
    INFO("Head Right");

    std::map<ServoMotorJoint, uint8_t> currentJointAngles = _servoController->get_current_joint_angles();

    int targetAngle = currentJointAngles[ServoMotorJoint::headLeftRight] - 1;
    if (targetAngle <= 0) {
        targetAngle = 0;
    }
    _servoController->set_absolute_servo_angle(ServoMotorJoint::headLeftRight, targetAngle);
    currentJointAngles[ServoMotorJoint::headLeftRight] = targetAngle;
}

void Robot::eye_up() {

}

void Robot::eye_down() {

}

