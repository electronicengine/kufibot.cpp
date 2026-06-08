#ifndef DISTANCE_CONTROLLER_H
#define DISTANCE_CONTROLLER_H

#include "controller.h"
#include "controller_data_structures.h"


class DistanceController : public Controller {
public:
    static DistanceController* get_instance();
    ~DistanceController() override;

    bool initialize() override;
    void shutdown() override;
    bool isReady() const noexcept override;

    bool is_active();
    DistanceData get_distance();
    void flush_input_buffer();

private:
    DistanceController();

    static DistanceController* _instance;

    int uart_fd = -1;                              // File descriptor for UART
};

#endif // DISTANCE_CONTROLLER_H