#include "compass_controller.h"


CompassController* CompassController::_instance = nullptr;

CompassController* CompassController::get_instance() {
    if (_instance == nullptr) {
        _instance = new CompassController();
    }
    return _instance;
}

double CompassController::get_angle() {
    return sensor.get_bearing();
}

std::vector<int16_t> CompassController::get_magnet() {
    return sensor.get_magnet();
}

const std::unordered_map<std::string, double> CompassController::get_all() {
    return {
        {"angle", get_angle()},
        {"magnet_x", static_cast<double>(get_magnet()[0])},
        {"magnet_y", static_cast<double>(get_magnet()[1])}
    };
}