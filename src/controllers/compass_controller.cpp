#include "compass_controller.h"


CompassController* CompassController::_instance = nullptr;

CompassController* CompassController::get_instance() {
    if (_instance == nullptr) {
        _instance = new CompassController();
    }
    return _instance;
}


uint16_t CompassController::get_angle() {
    int angle = sensor.get_bearing();
    angle = _medianFilter.apply(angle);
    angle += OFFSET_ANGLE;
    angle = angle % 360; 
    return angle;
}

std::vector<int16_t> CompassController::get_magnet() {
    return sensor.get_magnet();
}

CompassData CompassController::get_all() {
    std::vector<int16_t> magnet = get_magnet();
    return CompassData{get_angle(), magnet[0], magnet[1] };
}