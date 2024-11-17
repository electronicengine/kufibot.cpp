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

const std::map<std::string, double>& PowerController::get_consumption() {

    _data["busVoltage"] = ina.voltage();
    _data["shuntVoltage"] = ina.shunt_voltage();
    _data["current"] = ina.current();
    _data["power"] = ina.power();

    return _data;
}


