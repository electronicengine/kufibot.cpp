#ifndef POWER_CONTROLLER_H
#define POWER_CONTROLLER_H

#include <atomic>
#include "../drivers/ina219_driver.h"
#include "controller_data_structures.h"

#define SHUNT_OHMS 0.1
#define MAX_EXPECTED_AMPS 3.0


class PowerController {
public:
    static PowerController* get_instance();

    PowerData get_consumption();
    void setEnable(bool enable){ _enable.store(enable);}

private:
    INA219Driver ina; 
    static PowerController* _instance;
    std::atomic<bool> _enable = true;
    PowerController();
};

#endif //POWER_CONTROLLER_H
