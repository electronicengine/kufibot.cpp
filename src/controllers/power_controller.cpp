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
    data. shuntVoltage = ina.shunt_voltage();
    data. current = ina.current();
    data.power = ina.power();
    return data;
}


