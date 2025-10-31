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

#include "../logger.h"

PowerController* PowerController::_instance = nullptr;

PowerController* PowerController::get_instance() {
    if (_instance == nullptr) {
        _instance = new PowerController();
    }
    return _instance;
}

PowerController::PowerController() {
    bool ret = _ina.initINA219();
    if (ret) {
        _initialized = true;
        INFO("Power Controller initialized");
    }
    else{
        _initialized = false;
        ERROR("Power Controller initialization failed");
    }
}

PowerData PowerController::getConsumption() {
    if (!_enable.load() || !_initialized) {
        return PowerData{};
    }
    INA219Data data = _ina.readINA219();

    PowerData powerData;
    powerData.busVoltage = data.voltage;
    powerData.shuntVoltage = data.shunt_voltage;
    powerData.current = data.current;
    powerData.power = data.power;

    return powerData;
}


