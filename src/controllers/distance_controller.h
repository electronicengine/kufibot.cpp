#ifndef DISTANCE_CONTROLLER_H
#define DISTANCE_CONTROLLER_H

#include <boost/asio.hpp>
#include <iomanip> 
#include "../drivers/median_filter.h"

using namespace boost::asio;

// Parse the data and return as a struct
struct Data {
    int distance;
    int strength;
    int temperature;
};


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
    

public:
    bool is_active();
    static DistanceController* get_instance();
    ~DistanceController(); // Destructor to close the serial port
    const std::map<std::string, int> &get_distance();

   
};

#endif