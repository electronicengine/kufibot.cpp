#ifndef POWER_CONTROLLER_H
#define POWER_CONTROLLER_H

#include <iostream>
#include "../drivers/ina219_driver.h"
#include <unistd.h> // For usleep
#include <map>

#define SHUNT_OHMS 0.1
#define MAX_EXPECTED_AMPS 3.0

class PowerController {
public:
    static PowerController* get_instance();
    std::map<std::string, double> _data;

    const std::map<std::string, double>& get_consumption();

private:
    INA219Driver ina; 
    static PowerController* _instance;

    PowerController();
};

#endif //POWER_CONTROLLER_H
