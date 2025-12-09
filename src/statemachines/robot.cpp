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

    auto parser = JsonParserOperator::get_instance();
    auto jointPositionList = parser->getJointAngles();
    if (jointPositionList.has_value()) {
        _jointPositionList = jointPositionList.value();
    }else {
        ERROR("Joint Possions couldn't load!");
    }

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

SensorData Robot::getSensorValues() {
    std::lock_guard<std::mutex> lock(_controllerMutex);
    SensorData sensorData;

    sensorData.compassData = _compassController->getCompassData();
    sensorData.distanceData = _distanceController->get_distance();
    sensorData.powerData = _powerController->getConsumption();
    sensorData.dcMotorState = _dcMotorController->getCurrentState();
    sensorData.currentJointAngles = _servoController->getAllJointsAngle();

    return sensorData;
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
        control_eye(ServoMotorJoint::eyeRight, controlData.rightEye.value());
    }else if (controlData.jointAngle.has_value()) {
        _servoController->setJointAngle(controlData.jointAngle->first, controlData.jointAngle->second);
    }else if (controlData.jointAngles.has_value()) {
        std::map<ServoMotorJoint, uint8_t> jointAngles = _servoController->getAllJointsAngle();

        auto it = controlData.jointAngles->find(ServoMotorJoint::leftArm);
        if (it != controlData.jointAngles->end()) {
            if (it->second != jointAngles.at(ServoMotorJoint::leftArm)) {
                _servoController->setJointAngle(ServoMotorJoint::leftArm, controlData.jointAngles->at(ServoMotorJoint::leftArm));
            }
        }

        it = controlData.jointAngles->find(ServoMotorJoint::rightArm);
        if (it != controlData.jointAngles->end()) {
            if (it->second != jointAngles.at(ServoMotorJoint::rightArm)) {
                _servoController->setJointAngle(ServoMotorJoint::rightArm, controlData.jointAngles->at(ServoMotorJoint::rightArm));
            }
        }

        it = controlData.jointAngles->find(ServoMotorJoint::neck);
        if (it != controlData.jointAngles->end()) {
            if (it->second != jointAngles.at(ServoMotorJoint::neck)) {
                _servoController->setJointAngle(ServoMotorJoint::neck, controlData.jointAngles->at(ServoMotorJoint::neck));
            }
        }

        it = controlData.jointAngles->find(ServoMotorJoint::headUpDown);
        if (it != controlData.jointAngles->end()) {
            if (it->second != jointAngles.at(ServoMotorJoint::headUpDown)) {
                _servoController->setJointAngle(ServoMotorJoint::headUpDown, controlData.jointAngles->at(ServoMotorJoint::headUpDown));
            }
        }

        it = controlData.jointAngles->find(ServoMotorJoint::headLeftRight);
        if (it != controlData.jointAngles->end()) {
            if (it->second != jointAngles.at(ServoMotorJoint::headLeftRight)) {
                _servoController->setJointAngle(ServoMotorJoint::headLeftRight, controlData.jointAngles->at(ServoMotorJoint::headLeftRight));
            }
        }

        it = controlData.jointAngles->find(ServoMotorJoint::eyeLeft);
        if (it != controlData.jointAngles->end()) {
            if (it->second != jointAngles.at(ServoMotorJoint::eyeLeft)) {
                _servoController->setJointAngle(ServoMotorJoint::eyeLeft, controlData.jointAngles->at(ServoMotorJoint::eyeLeft));
            }
        }

        it = controlData.jointAngles->find(ServoMotorJoint::eyeRight);
        if (it != controlData.jointAngles->end()) {
            if (it->second != jointAngles.at(ServoMotorJoint::eyeRight)) {
                _servoController->setJointAngle(ServoMotorJoint::eyeRight, controlData.jointAngles->at(ServoMotorJoint::eyeRight));
            }
        }
    }

}

void Robot::control_body(int angle, int magnitude) {

    if (!_dcMotorController) {
        return;
    }

    if (magnitude >= 100) {
        magnitude = 95;
    }

    if (magnitude == 0 && angle == 0) {
        _dcMotorController->stop();
        return;
    }

    // Normalize angle to [-180, 180]
    angle = ((angle + 180) % 360) - 180;

    if (angle >= UP_MIN && angle < UP_MAX) {
        _dcMotorController->forward(magnitude);
    } else if (angle >= RIGHT_MIN && angle < RIGHT_MAX) {
        _dcMotorController->turnRight(magnitude);
    } else if (angle >= DOWN_MIN && angle < DOWN_MAX) {
        _dcMotorController->backward(magnitude);
    } else if (angle >= LEFT_MIN || angle < LEFT_MAX) {
        _dcMotorController->turnLeft(magnitude);
    } else if (angle == 0 || magnitude == 0) {
        _dcMotorController->stop();  // Optional: fallback safety
    } else {
        WARNING("control_body: unknown angle: {} - magnitude: {}", std::to_string(angle), std::to_string(magnitude));
    }
}

void Robot::control_head(int angle, int magnitude) {
    if (!_servoController || (magnitude == 0 && angle == 0)) {
        return;
    }

    if (magnitude > 5) {
        magnitude = 5;
    }

    auto upDown = sin(angle * M_PI / 180.0);
    int magUpDown = abs(upDown * magnitude);
    if (upDown > 0.001) {
        head_up(magUpDown);
    } else if (upDown < -0.001) {
        head_down(magUpDown);
    }

    auto leftRight = cos(angle * M_PI / 180.0);
    int magLeftRight = abs(leftRight * magnitude);

    if (leftRight > 0.001) {
        head_right(magLeftRight);
    } else if (leftRight < -0.001) {
        head_left(magLeftRight);
    }
}


void Robot::control_arm(ServoMotorJoint joint, int angle, bool scale) const {

    if(_servoController){
        double mapped_angle;
        if(scale){
            mapped_angle = (angle / 100.0) * 180.0;
        }else {
            mapped_angle = angle;
        }

        _servoController->setJointAngle(joint, mapped_angle);
    }
}


void Robot::control_eye(ServoMotorJoint joint, bool state) {

    INFO("Control Eye: {} - {}", (int)joint, state);
    if(_servoController){
        if (joint == ServoMotorJoint::eyeLeft) {
            if (state)
                _servoController->setJointAngle(ServoMotorJoint::eyeLeft, _jointPositionList[ServoMotorJoint::eyeLeft][GestureJointState::fullup].angle);
            else
                _servoController->setJointAngle(ServoMotorJoint::eyeLeft, _jointPositionList[ServoMotorJoint::eyeLeft][GestureJointState::fulldown].angle);

            //eyes Up
        } else if (joint == ServoMotorJoint::eyeRight) {
            if (state)
                _servoController->setJointAngle(ServoMotorJoint::eyeRight, _jointPositionList[ServoMotorJoint::eyeRight][GestureJointState::fullup].angle);
            else
                _servoController->setJointAngle(ServoMotorJoint::eyeRight, _jointPositionList[ServoMotorJoint::eyeRight][GestureJointState::fulldown].angle);
        }
    }
}


void Robot::head_down(int speed) {
    if (speed < 1)
        speed = 1;

    std::map<ServoMotorJoint, uint8_t> currentJointAngles = _servoController->getAllJointsAngle();
    int targetAngle = currentJointAngles[ServoMotorJoint::headUpDown] + 1 * speed;
    if (targetAngle >= _jointPositionList[ServoMotorJoint::headUpDown][GestureJointState::fulldown].angle) {
        targetAngle = _jointPositionList[ServoMotorJoint::headUpDown][GestureJointState::fulldown].angle;
    }
    _servoController->setJointAngle(ServoMotorJoint::headUpDown, targetAngle);
    currentJointAngles[ServoMotorJoint::headUpDown] = targetAngle;

    targetAngle = currentJointAngles[ServoMotorJoint::neck] - 1 * speed;
    if (targetAngle <= _jointPositionList[ServoMotorJoint::neck][GestureJointState::fulldown].angle) {
        targetAngle = _jointPositionList[ServoMotorJoint::neck][GestureJointState::fulldown].angle;
    }
    _servoController->setJointAngle(ServoMotorJoint::neck, targetAngle);
    currentJointAngles[ServoMotorJoint::neck] = targetAngle;
}

void Robot::head_up(int speed) {
    if (speed < 1)
        speed = 1;

    std::map<ServoMotorJoint, uint8_t> currentJointAngles = _servoController->getAllJointsAngle();

    int targetAngle = currentJointAngles[ServoMotorJoint::headUpDown] - 1 * speed;
    if (targetAngle <= _jointPositionList[ServoMotorJoint::headUpDown][GestureJointState::fullup].angle) {
        targetAngle = _jointPositionList[ServoMotorJoint::headUpDown][GestureJointState::fullup].angle;
    }
    _servoController->setJointAngle(ServoMotorJoint::headUpDown, targetAngle);
    currentJointAngles[ServoMotorJoint::headUpDown] = targetAngle;

    targetAngle = currentJointAngles[ServoMotorJoint::neck] + 1 * speed;
    if (targetAngle >= _jointPositionList[ServoMotorJoint::neck][GestureJointState::fullup].angle) {
        targetAngle = _jointPositionList[ServoMotorJoint::neck][GestureJointState::fullup].angle;
    }
    _servoController->setJointAngle(ServoMotorJoint::neck, targetAngle);
    currentJointAngles[ServoMotorJoint::neck] = targetAngle;
}

void Robot::head_left(int speed) {
    if (speed < 1)
        speed = 1;

    std::map<ServoMotorJoint, uint8_t> currentJointAngles = _servoController->getAllJointsAngle();

    int targetAngle = currentJointAngles[ServoMotorJoint::headLeftRight] + 1 * speed;
    if (targetAngle >= 180) {
        targetAngle = 180;
    }
    _servoController->setJointAngle(ServoMotorJoint::headLeftRight, targetAngle);
    currentJointAngles[ServoMotorJoint::headLeftRight] = targetAngle;
}

void Robot::head_right(int speed) {
    if (speed < 1)
        speed = 1;

    std::map<ServoMotorJoint, uint8_t> currentJointAngles = _servoController->getAllJointsAngle();

    int targetAngle = currentJointAngles[ServoMotorJoint::headLeftRight] - 1 * speed;
    if (targetAngle <= 0) {
        targetAngle = 0;
    }
    _servoController->setJointAngle(ServoMotorJoint::headLeftRight, targetAngle);
    currentJointAngles[ServoMotorJoint::headLeftRight] = targetAngle;
}

void Robot::eye_up() {

}

void Robot::eye_down() {

}

