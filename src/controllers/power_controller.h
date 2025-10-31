#ifndef POWER_CONTROLLER_H
#define POWER_CONTROLLER_H

#include <atomic>
#include "../drivers/ina219_driver.h"
#include "controller_data_structures.h"

class PowerController {
public:
    static PowerController* get_instance();

    PowerData getConsumption();
    void setEnable(bool enable){ _enable.store(enable);}

private:
    INA219Driver _ina;
    std::atomic<bool> _initialized = false;
    static PowerController* _instance;
    std::atomic<bool> _enable = true;
    PowerController();
};

#endif //POWER_CONTROLLER_H
