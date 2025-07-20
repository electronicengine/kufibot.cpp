#include "robot_controller_service.h"
#include "gesture_performing_service.h"
#include "mapping_service.h"
#include "tui_service.h"

#include "../logger.h"

RobotControllerService* RobotControllerService::_instance = nullptr;

RobotControllerService *RobotControllerService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new RobotControllerService();
    }
    return _instance;
}


RobotControllerService::RobotControllerService() : Service("RobotControllerService")
{

}

void RobotControllerService::service_function() {

    _compassController =  CompassController::get_instance();
    _distanceController = DistanceController::get_instance();
    _powerController = PowerController::get_instance();
    _servoController = ServoMotorController::get_instance();
    _dcMotorController = DCMotorController::get_instance();

    subscribe_to_service(MappingService::get_instance());
    subscribe_to_service(GesturePerformingService::get_instance());
    subscribe_to_service(TuiService::get_instance());

    while (_running) {
        std::this_thread::sleep_for(std::chrono::microseconds(200));
        if(!_subscribers.empty()){
            std::unique_ptr<MessageData> data = std::make_unique<SensorData>();
            *static_cast<SensorData*>(data.get()) = get_sensor_values();
            publish(MessageType::SensorData, data);
        }
    }
}

RobotControllerService::~RobotControllerService()
{
}



SensorData RobotControllerService::get_sensor_values()
{
    SensorData sensorData;
    sensorData.compassData = _compassController->get_all();
    sensorData.distanceData = _distanceController->get_distance();
    sensorData.powerData = _powerController->get_consumption();
    sensorData.currentJointAngles = _servoController->get_current_joint_angles();
    sensorData.dcMotorState = _dcMotorController->get_current_state();

    return sensorData;
}

void RobotControllerService::control_motion(const ControlData& controlData)
{
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
        _servoController->set_absolute_servo_angle(ServoMotorJoint::leftArm, controlData.jointAngles->at(ServoMotorJoint::leftArm));
        _servoController->set_absolute_servo_angle(ServoMotorJoint::rightArm, controlData.jointAngles->at(ServoMotorJoint::rightArm));
        _servoController->set_absolute_servo_angle(ServoMotorJoint::neck, controlData.jointAngles->at(ServoMotorJoint::neck));
        _servoController->set_absolute_servo_angle(ServoMotorJoint::headUpDown, controlData.jointAngles->at(ServoMotorJoint::headUpDown));
        _servoController->set_absolute_servo_angle(ServoMotorJoint::headLeftRight, controlData.jointAngles->at(ServoMotorJoint::headLeftRight));
        _servoController->set_absolute_servo_angle(ServoMotorJoint::eyeLeft, controlData.jointAngles->at(ServoMotorJoint::eyeLeft));
        _servoController->set_absolute_servo_angle(ServoMotorJoint::eyeRight, controlData.jointAngles->at(ServoMotorJoint::eyeRight));
    }
}

void RobotControllerService::control_body(int angle, int magnitude) {

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

void RobotControllerService::control_head(int angle, int magnitude) {
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

void RobotControllerService::control_arm(ServoMotorJoint joint, int angle, bool scale) const {
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


void RobotControllerService::control_eye(ServoMotorJoint joint, bool state) {

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

void RobotControllerService::subcribed_data_receive(MessageType type,  const std::unique_ptr<MessageData>& data) {
    std::lock_guard<std::mutex> lock(_dataMutex);

    switch (type) {
        case MessageType::ControlData: {
            if (data) {
                INFO("RobotControllerService::subcribed_data_receive-ControlData");

                ControlData controlData = *static_cast<ControlData*>(data.get());
                control_motion(controlData);
            }
            break;
        }
        default:
            WARNING("{} subcribed_data_receive unknown message type!", get_service_name());
            break;
    }
}


void RobotControllerService::head_down() {
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

void RobotControllerService::head_up() {
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

void RobotControllerService::head_left() {
    INFO("Head Left");

    std::map<ServoMotorJoint, uint8_t> currentJointAngles = _servoController->get_current_joint_angles();

    int targetAngle = currentJointAngles[ServoMotorJoint::headLeftRight] + 1;
    if (targetAngle >= 180) {
        targetAngle = 180;
    }
    _servoController->set_absolute_servo_angle(ServoMotorJoint::headLeftRight, targetAngle);
    currentJointAngles[ServoMotorJoint::headLeftRight] = targetAngle;
}

void RobotControllerService::head_right() {
    INFO("Head Right");

    std::map<ServoMotorJoint, uint8_t> currentJointAngles = _servoController->get_current_joint_angles();

    int targetAngle = currentJointAngles[ServoMotorJoint::headLeftRight] - 1;
    if (targetAngle <= 0) {
        targetAngle = 0;
    }
    _servoController->set_absolute_servo_angle(ServoMotorJoint::headLeftRight, targetAngle);
    currentJointAngles[ServoMotorJoint::headLeftRight] = targetAngle;
}

void RobotControllerService::eye_up() {

}

void RobotControllerService::eye_down() {

}


