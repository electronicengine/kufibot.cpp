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

#include "../logger.h"


CompassController *CompassController::_instance = nullptr;

 CompassController::CompassController() {
    bool ret = sensor.initQMC5883l();
     if (ret) {
         _initialized = true;
         INFO("Compass Controller initialized");
     }else {
         _initialized = false;
         ERROR("Compass Controller initialization failed");
     }
 }

 CompassController * CompassController::get_instance() {
    if (_instance == nullptr) {
        _instance = new CompassController();
    }
    return _instance;
}


CompassData CompassController::getCompassData() {
    CompassData data;

     if (!_enable || !_initialized) {
         return data;
     }

    Axis compasVector = sensor.readQMC5883l();
    // Compute angle in radians
    double angle_rad = std::atan2(compasVector.x, compasVector.y);

    // Convert to degrees
    double angle_deg = angle_rad * 180.0 / M_PI;

     data.angle = angle_deg;
     data.magnetX = compasVector.x;
     data.magnetY = compasVector.y;

    return data;
}
