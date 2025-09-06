#ifndef DISTANCE_CONTROLLER_H
#define DISTANCE_CONTROLLER_H

#include <boost/asio.hpp>
#include <iomanip> 
#include "../drivers/median_filter.h"
#include "controller_data_structures.h"

using namespace boost::asio;


class DistanceController {
private:
    static DistanceController* _instance;
    io_service io;
    serial_port uart;
    std::map<std::string, int> _sensorData;
    MedianFilter _medianFilter;

    double _distance; 
    DistanceController(); // Private constructor for singleton pattern
    void flush_input_buffer();
    std::atomic<bool> _enable = true;

public:
    bool is_active();
    static DistanceController* get_instance();
    ~DistanceController(); // Destructor to close the serial port
    DistanceData get_distance();
    void setEnable(bool enable){ _enable.store(enable);}

   
};

#endif