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