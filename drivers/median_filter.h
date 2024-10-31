
#ifndef MEDIAN_FILTER_H
#define MEDIAN_FILTER_H

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <numeric>
#include <vector>
#include <algorithm>
#include "../drivers/qmc5883l_driver.h"

#define OFFSET_ANGLE 79

class MedianFilter {
public:
    MedianFilter(size_t window_size);

    double apply(double new_value);

private:
    std::vector<double> values; // Store the sensor values
    size_t window_size;

    double compute_median();
};

#endif