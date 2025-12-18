
#ifndef COMPASS_CONTROLLER_H
#define COMPASS_CONTROLLER_H

#include <atomic>
#include <vector>
#include "controller_data_structures.h"
#include "../drivers/median_filter.h"
#include "../drivers/qmc5883l_driver.h"
#include "../drivers/hmc5883l_driver.h"

#define OFFSET_ANGLE 79

class CompassController {
private:
    static CompassController* _instance; // Singleton instance
    // QMC5883LDriver sensor;            // Sensor object
    HMC5883LDriver sensor;
    std::atomic<bool> _enable = true;
    std::atomic<bool> _initialized = false;

    CompassController();

public:
    // Delete copy constructor and assignment operator
    CompassController(const CompassController&) = delete;
    CompassController& operator=(const CompassController&) = delete;

    // Get singleton instance
    static CompassController* get_instance();
    CompassData getCompassData();

    void setEnable(bool enable){_enable.store(enable);}

};

#endif