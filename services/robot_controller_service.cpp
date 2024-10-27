#include "robot_controller_service.h"

RobotControllerService* RobotControllerService::_instance = nullptr;


RobotControllerService *RobotControllerService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new RobotControllerService();
    }
    return _instance;
}


RobotControllerService::RobotControllerService()
{
    _compassController =  CompassController::get_instance();
    _distanceController = DistanceController::get_instance();
    _powerController = PowerController::get_instance();
    _servoController = ServoMotorController::get_instance();
    _dcMotorController = DCMotorController::get_instance();
}

RobotControllerService::~RobotControllerService()
{
}



void RobotControllerService::update_sensors() {
    while (_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1)); 
        if(!_subscribers.empty()){
            _sensor_values = get_sensor_values(); 
            update_sensor_values(_sensor_values);
        }
    }
}


Json RobotControllerService::get_sensor_values()
{
    const std::unordered_map<std::string, double>& compass_map = _compassController->get_all();
    const std::map<std::string, int>& distance_map = _distanceController->get_dinstance();
    const std::map<std::string, double>& power_map = _powerController->get_consumption();
    const std::map<std::string, int> joints_map = _servoController->get_all_joint_angles();

    Json joint_angles = {
        {"right_arm", joints_map.at("right_arm")},
        {"left_arm", joints_map.at("left_arm")},
        {"neck_down", joints_map.at("neck_down")},
        {"neck_up", joints_map.at("neck_up")},
        {"neck_right", joints_map.at("neck_right")},
        {"eye_right", joints_map.at("eye_right")},
        {"eye_left", joints_map.at("eye_left")}
    };

    Json compass = {
        {"angle", compass_map.at("angle")},
        {"magnet", {
            {"magnet_x", compass_map.at("magnet_x")},
            {"magnet_y", compass_map.at("magnet_y")}
        }}
    };

    Json distance = {
        {"Distance", distance_map.at("distance")},
        {"Strength", distance_map.at("strength")},
        {"Temperature", distance_map.at("temperature")}
    };

    Json power = {
        {"BusVoltage", power_map.at("busVoltage")},
        {"BusCurrent", power_map.at("current")}, // Fixed key for BusCurrent
        {"Power", power_map.at("power")},
        {"ShuntVoltage", power_map.at("shuntVoltage")}
    };

    Json metadata = {
        {"joint_angles", joint_angles},
        {"compass", compass},
        {"distance", distance},
        {"power", power}
    };

    return metadata;
}

int RobotControllerService::control_motion(Json message)
{
    if (!message.is_null()) {
        std::string object_name = message.begin().key();
        std::string control_id = object_name;

        if (control_id == "body_joystick") {
            int angle = message[object_name]["Angle"];
            int strength = message[object_name]["Strength"];
            control_body(angle, strength);
        } else if (control_id == "head_joystick") {
            int angle = message[object_name]["Angle"];
            int strength = message[object_name]["Strength"];
            control_head(angle, strength);
        } else if (control_id == "right_arm" || control_id == "left_arm") {
            int angle = message[object_name]["Angle"];
            control_arm(control_id, angle);
        } else if (control_id == "right_eye" || control_id == "left_eye") {
            int angle = message[object_name]["Angle"];
            control_eye(angle);
        } else {
        }
    }

    return 0;
}

void RobotControllerService::control_body(int angle, int magnitude) {
    if (magnitude == 0 && angle == 0) {
        _dcMotorController->stop();
    } else if (45 <= angle && angle < 135) {
         _dcMotorController->forward(magnitude);
    } else if (-45 <= angle && angle < 45) {
         _dcMotorController->turn_right(magnitude);
    } else if (-135 <= angle && angle < -45) {
         _dcMotorController->backward(magnitude);
    } else if (angle >= 135 || angle < -135) {
         _dcMotorController->turn_left(magnitude);
    } else {
         _dcMotorController->stop();
    }
}

void RobotControllerService::control_head(int angle, int magnitude) {
    if (magnitude == 0 && angle == 0) {
        return;
    } else if (45 <= angle && angle < 135) {
        _servoController->head_up();
    } else if (-45 <= angle && angle < 45) {
        _servoController->head_right();
    } else if (-135 <= angle && angle < -45) {
        _servoController->head_down(); 
    } else if (angle >= 135 || angle < -135) {
        _servoController->head_left();
    }
}

void RobotControllerService::control_arm(const std::string& control_id, int angle) {
    double mapped_angle = (angle / 100.0) * 180.0;
    if (control_id == "left_arm") {
        _servoController->set_absolute_servo_angle("left_arm", mapped_angle);
    } else if (control_id == "right_arm") {
        _servoController->set_absolute_servo_angle("right_arm", mapped_angle); 
    }
}

void RobotControllerService::control_eye(int angle) {
    if (angle == 180) {
        _servoController->eye_up();
    } else if (angle == 0) {
        _servoController->eye_down();
    }
}

void RobotControllerService::start()
{
    if (!_running) { // Ensure the thread is not already running
        _running = true;
        std::cout << "RobotControllerService is starting..." << std::endl;
        _sensor_thread = std::thread(&RobotControllerService::update_sensors, this);
    }
}

void RobotControllerService::stop()
{
    if (_running){
        _running = false;
        std::cout << "RobotControllerService is stopping..." << std::endl;
        if (_sensor_thread.joinable()) {
            _sensor_thread.join(); 
        }
        std::cout << "RobotControllerService is stopped." << std::endl;
    }
}
