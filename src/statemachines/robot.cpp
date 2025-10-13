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
#include "../operators/json_parser_operator.h"

// Robot
Robot::Robot() {
    idleState = createState<IdleState>("IdleState");
    movingState = createState<MovingState>("MovingState");
    criticalErrorState = createState<CriticalErrorState>("CriticalErrorState");
    initializeState = createState<InitializeState>("InitializeState");

    tuiControlState = createState<TuiControlState>("TuiControlState", movingState);
    remoteControlState = createState<RemoteControlState>("RemoteControlState", movingState);
    talkingState = createState<TalkingState>("TalkingState", movingState);
    trackingState = createState<TrackingState>("TrackingState", movingState);

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

    _jointLimits = JsonParserOperator::getJointLimits("/usr/local/etc/joint_angles.json");

    return true;
}

void Robot::setEnableSensorContinuousReadings(bool enable) {

    WARNING("setEnableSensorContinuousReadings {}", enable);

    _compassController->setEnable(enable);
    _distanceController->setEnable(enable);
    _powerController->setEnable(enable);
    _servoController->setEnable(!enable);
    _dcMotorController->setEnable(!enable);

    _enableSensorContinuousReadings = enable;
}

SensorData Robot::getCurrentMotorPositions() {
    std::lock_guard<std::mutex> lock(_controllerMutex);
    SensorData sensorData;

    sensorData.dcMotorState = _dcMotorController->get_current_state();
    sensorData.currentJointAngles = _servoController->get_current_joint_angles();

    return sensorData;
}

SensorData Robot::get_sensor_values()
{
    std::lock_guard<std::mutex> lock(_controllerMutex);

    SensorData sensorData;

    if (_enableSensorContinuousReadings) {
        sensorData.compassData = _compassController->get_all();
        sensorData.distanceData = _distanceController->get_distance();
        sensorData.powerData = _powerController->get_consumption();
        sensorData.currentJointAngles = _servoController->get_current_joint_angles();
        sensorData.dcMotorState = _dcMotorController->get_current_state();

        return sensorData;
    }else {
        return sensorData;
    }

}

void Robot::control_motion(const ControlData& controlData)
{
    std::lock_guard<std::mutex> lock(_controllerMutex);

    INFO("control_motion");

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

        auto it = controlData.jointAngles->find(ServoMotorJoint::leftArm);
        if (it != controlData.jointAngles->end()) {
            if (it->second != jointAngles.at(ServoMotorJoint::leftArm)) {
                INFO("setting leftArm absolute servo angle: {}", controlData.jointAngles->at(ServoMotorJoint::leftArm));
                _servoController->set_absolute_servo_angle(ServoMotorJoint::leftArm, controlData.jointAngles->at(ServoMotorJoint::leftArm));
            }
        }

        it = controlData.jointAngles->find(ServoMotorJoint::rightArm);
        if (it != controlData.jointAngles->end()) {
            if (it->second != jointAngles.at(ServoMotorJoint::rightArm)) {
                INFO("setting rightArm absolute servo angle: {}", controlData.jointAngles->at(ServoMotorJoint::rightArm));
                _servoController->set_absolute_servo_angle(ServoMotorJoint::rightArm, controlData.jointAngles->at(ServoMotorJoint::rightArm));
            }
        }

        it = controlData.jointAngles->find(ServoMotorJoint::neck);
        if (it != controlData.jointAngles->end()) {
            if (it->second != jointAngles.at(ServoMotorJoint::neck)) {
                INFO("setting neck absolute servo angle: {}", controlData.jointAngles->at(ServoMotorJoint::neck));
                _servoController->set_absolute_servo_angle(ServoMotorJoint::neck, controlData.jointAngles->at(ServoMotorJoint::neck));
            }
        }

        it = controlData.jointAngles->find(ServoMotorJoint::headUpDown);
        if (it != controlData.jointAngles->end()) {
            if (it->second != jointAngles.at(ServoMotorJoint::headUpDown)) {
                INFO("setting headUpDown absolute servo angle: {}", controlData.jointAngles->at(ServoMotorJoint::headUpDown));
                _servoController->set_absolute_servo_angle(ServoMotorJoint::headUpDown, controlData.jointAngles->at(ServoMotorJoint::headUpDown));
            }
        }

        it = controlData.jointAngles->find(ServoMotorJoint::headLeftRight);
        if (it != controlData.jointAngles->end()) {
            if (it->second != jointAngles.at(ServoMotorJoint::headLeftRight)) {
                INFO("setting headLeftRight absolute servo angle: {}", controlData.jointAngles->at(ServoMotorJoint::headLeftRight));
                _servoController->set_absolute_servo_angle(ServoMotorJoint::headLeftRight, controlData.jointAngles->at(ServoMotorJoint::headLeftRight));
            }
        }

        it = controlData.jointAngles->find(ServoMotorJoint::eyeLeft);
        if (it != controlData.jointAngles->end()) {
            if (it->second != jointAngles.at(ServoMotorJoint::eyeLeft)) {
                INFO("setting eyeLeft absolute servo angle: {}", controlData.jointAngles->at(ServoMotorJoint::eyeLeft));
                _servoController->set_absolute_servo_angle(ServoMotorJoint::eyeLeft, controlData.jointAngles->at(ServoMotorJoint::eyeLeft));
            }
        }

        it = controlData.jointAngles->find(ServoMotorJoint::eyeRight);
        if (it != controlData.jointAngles->end()) {
            if (it->second != jointAngles.at(ServoMotorJoint::eyeRight)) {
                INFO("setting eyeRight absolute servo angle: {}", controlData.jointAngles->at(ServoMotorJoint::eyeRight));
                _servoController->set_absolute_servo_angle(ServoMotorJoint::eyeRight, controlData.jointAngles->at(ServoMotorJoint::eyeRight));
            }
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
    } else {
        WARNING("control_body: unknown angle: {} - magnitude: {}", std::to_string(angle), std::to_string(magnitude));
    }
}

void Robot::control_head(int angle, int magnitude) {
    INFO("Control head: {} - magnitude: {}", std::to_string(angle), std::to_string(magnitude));

    if (!_servoController || (magnitude == 0 && angle == 0)) {
        return;
    }

    auto upDown = sin(angle * M_PI / 180.0);
    int magUpDown = abs(upDown * magnitude);
    INFO("upDown: {} - {}", upDown, magUpDown);
    if (upDown > 0.001) {
        head_up(magUpDown);
    } else if (upDown < -0.001) {
        head_down(magUpDown);
    }

    auto leftRight = cos(angle * M_PI / 180.0);
    int magLeftRight = abs(leftRight * magnitude);
    INFO("leftRight: {} - {}", leftRight, magLeftRight);

    if (leftRight > 0.001) {
        head_right(magLeftRight);
    } else if (leftRight < -0.001) {
        head_left(magLeftRight);
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


void Robot::head_down(int speed) {
    INFO("Head Down");
    if (speed < 1)
        speed = 1;

    std::map<ServoMotorJoint, uint8_t> currentJointAngles = _servoController->get_current_joint_angles();
    int targetAngle = currentJointAngles[ServoMotorJoint::headUpDown] + 1 * speed;
    if (targetAngle >= _jointLimits[ServoMotorJoint::headUpDown][GestureJointState::fulldown].angle) {
        targetAngle = _jointLimits[ServoMotorJoint::headUpDown][GestureJointState::fulldown].angle;
    }
    _servoController->set_absolute_servo_angle(ServoMotorJoint::headUpDown, targetAngle);
    currentJointAngles[ServoMotorJoint::headUpDown] = targetAngle;

    targetAngle = currentJointAngles[ServoMotorJoint::neck] - 1 * speed;
    if (targetAngle <= _jointLimits[ServoMotorJoint::neck][GestureJointState::fulldown].angle) {
        targetAngle = _jointLimits[ServoMotorJoint::neck][GestureJointState::fulldown].angle;
    }
    _servoController->set_absolute_servo_angle(ServoMotorJoint::neck, targetAngle);
    currentJointAngles[ServoMotorJoint::neck] = targetAngle;
}

void Robot::head_up(int speed) {
    INFO("Head Up");
    if (speed < 1)
        speed = 1;

    std::map<ServoMotorJoint, uint8_t> currentJointAngles = _servoController->get_current_joint_angles();

    int targetAngle = currentJointAngles[ServoMotorJoint::headUpDown] - 1 * speed;
    if (targetAngle <= _jointLimits[ServoMotorJoint::headUpDown][GestureJointState::fullup].angle) {
        targetAngle = _jointLimits[ServoMotorJoint::headUpDown][GestureJointState::fullup].angle;
    }
    _servoController->set_absolute_servo_angle(ServoMotorJoint::headUpDown, targetAngle);
    currentJointAngles[ServoMotorJoint::headUpDown] = targetAngle;

    targetAngle = currentJointAngles[ServoMotorJoint::neck] + 1 * speed;
    if (targetAngle >= _jointLimits[ServoMotorJoint::neck][GestureJointState::fullup].angle) {
        targetAngle = _jointLimits[ServoMotorJoint::neck][GestureJointState::fullup].angle;
    }
    _servoController->set_absolute_servo_angle(ServoMotorJoint::neck, targetAngle);
    currentJointAngles[ServoMotorJoint::neck] = targetAngle;
}

void Robot::head_left(int speed) {
    INFO("Head Left");
    if (speed < 1)
        speed = 1;

    std::map<ServoMotorJoint, uint8_t> currentJointAngles = _servoController->get_current_joint_angles();

    int targetAngle = currentJointAngles[ServoMotorJoint::headLeftRight] + 1 * speed;
    if (targetAngle >= 180) {
        targetAngle = 180;
    }
    _servoController->set_absolute_servo_angle(ServoMotorJoint::headLeftRight, targetAngle);
    currentJointAngles[ServoMotorJoint::headLeftRight] = targetAngle;
}

void Robot::head_right(int speed) {
    INFO("Head Right");
    if (speed < 1)
        speed = 1;

    std::map<ServoMotorJoint, uint8_t> currentJointAngles = _servoController->get_current_joint_angles();

    int targetAngle = currentJointAngles[ServoMotorJoint::headLeftRight] - 1 * speed;
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

