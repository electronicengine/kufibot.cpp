#include "median_filter.h"

MedianFilter::MedianFilter(size_t window_size) : window_size(window_size) {}

double MedianFilter::apply(double new_value) {
    // Add the new value to the window
    values.push_back(new_value);
    
    // Maintain the window size
    if (values.size() > window_size) {
        values.erase(values.begin());
    }
    
    // Compute and return the median
    return compute_median();
}

double MedianFilter::compute_median() {
    // Make a copy of the values to sort
    std::vector<double> sorted_values = values;
    std::sort(sorted_values.begin(), sorted_values.end());

    // Calculate the median
    size_t n = sorted_values.size();
    if (n % 2 == 0) {
        // If even, average the two middle elements
        return (sorted_values[n / 2 - 1] + sorted_values[n / 2]) / 2.0;
    } else {
        // If odd, return the middle element
        return sorted_values[n / 2];
    }
}

