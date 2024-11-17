
#ifndef COMPASS_CONTROLLER_H
#define COMPASS_CONTROLLER_H

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <numeric>
#include <vector>
#include <algorithm>
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

public:
    // Delete copy constructor and assignment operator
    CompassController(const CompassController&) = delete;
    CompassController& operator=(const CompassController&) = delete;

    // Get singleton instance
    static CompassController* get_instance();
    double get_angle();
    std::vector<int16_t> get_magnet();
    const std::unordered_map<std::string, double> get_all() ;
};


#endif