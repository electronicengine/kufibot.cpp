#ifndef DISTANCE_CONTROLLER_H
#define DISTANCE_CONTROLLER_H

#include <atomic>
#include <cstdint>
#include "controller_data_structures.h"


class DistanceController {
public:
    static DistanceController* get_instance();
    ~DistanceController();

    bool is_active();
    DistanceData get_distance();
    void flush_input_buffer();
    void setEnable(bool enable){ _enable.store(enable);}

private:
    DistanceController();

    static DistanceController* _instance;
    std::atomic<bool> _enable = true;
    bool _initialized = false;

    int uart_fd = -1;                              // File descriptor for UART
};

#endif // DISTANCE_CONTROLLER_H