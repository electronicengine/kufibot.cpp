#ifndef POWER_CONTROLLER_H
#define POWER_CONTROLLER_H

#include "controller.h"
#include "../drivers/ina219_driver.h"
#include "controller_data_structures.h"

class PowerController : public Controller {
public:
    static PowerController* get_instance();
    ~PowerController() override;

    bool initialize() override;
    void shutdown() override;
    bool isReady() const noexcept override;

    PowerData getConsumption();

private:
    INA219Driver _ina;
    static PowerController* _instance;
    PowerController();
};

#endif //POWER_CONTROLLER_H
