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
#include "power_controller.h"

PowerController* PowerController::_instance = nullptr;

PowerController* PowerController::get_instance() {
    if (_instance == nullptr) {
        _instance = new PowerController();
    }
    return _instance;
}

PowerController::PowerController() : ina(SHUNT_OHMS, MAX_EXPECTED_AMPS) {
    ina.configure(RANGE_16V, GAIN_8_320MV, ADC_12BIT, ADC_12BIT); 
} 

PowerData PowerController::get_consumption() {

    PowerData data;
    data.busVoltage = ina.voltage();
    data.shuntVoltage = ina.shunt_voltage();
    data.current = ina.current();
    data.power = ina.power();
    return data;
}


