#ifndef POWER_CONTROLLER_H
#define POWER_CONTROLLER_H

#include "../drivers/ina219_driver.h"
#include "controller_data_structures.h"

#define SHUNT_OHMS 0.1
#define MAX_EXPECTED_AMPS 3.0


class PowerController {
public:
    static PowerController* get_instance();

    PowerData get_consumption();

private:
    INA219Driver ina; 
    static PowerController* _instance;

    PowerController();
};

#endif //POWER_CONTROLLER_H
