
#ifndef COMPASS_CONTROLLER_H
#define COMPASS_CONTROLLER_H

#include <vector>
#include "controller_data_structures.h"
#include "../drivers/median_filter.h"
#include "../drivers/qmc5883l_driver.h"

#define OFFSET_ANGLE 79

class CompassController {
private:
    static CompassController* _instance; // Singleton instance
    QMC5883LDriver sensor;            // Sensor object
    MedianFilter _medianFilter; // Instance of the median filter

    // Private constructor to prevent instantiation
    CompassController(): _medianFilter(10) {}
    CompassData get_all_data();

public:
    // Delete copy constructor and assignment operator
    CompassController(const CompassController&) = delete;
    CompassController& operator=(const CompassController&) = delete;

    // Get singleton instance
    static CompassController* get_instance();
    uint16_t get_angle();
    std::vector<int16_t> get_magnet();
    CompassData get_all() ;
};


#endif