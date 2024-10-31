#include "distance_controller.h"


DistanceController* DistanceController::_instance = nullptr;

DistanceController* DistanceController::get_instance() {
    if (_instance == nullptr) {
        _instance = new DistanceController();
    }
    return _instance;
}

DistanceController::DistanceController() : uart(io, "/dev/ttyAMA0"), _medianFilter(10)  {
    uart.set_option(serial_port_base::baud_rate(115200));
    uart.set_option(serial_port_base::character_size(8));
    uart.set_option(serial_port_base::parity(serial_port_base::parity::none));
    uart.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
    uart.set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));
}

DistanceController::~DistanceController(){
    if (uart.is_open()) {
        uart.close();
    }
}


void DistanceController::flush_input_buffer() {
    int fd = uart.native_handle();  // Get the native file descriptor
    tcflush(fd, TCIFLUSH);          // Flush the input buffer
}


const std::map<std::string, int> &DistanceController::get_distance() {
    std::vector<uint8_t> buffer(9);
    boost::system::error_code ec;
    
    // Read 9 bytes from the serial port
    size_t bytes_read = boost::asio::read(uart, boost::asio::buffer(buffer), ec);

    if (ec || bytes_read != 9) {
        std::cerr << "Failed to read data: " << ec.message() << std::endl;
        return _sensorData;  // Error case
    }

    if (buffer[0] == 'Y' && buffer[1] == 'Y') {
        int distL = static_cast<int>(buffer[2]);
        int distH = static_cast<int>(buffer[3]);
        int stL = static_cast<int>(buffer[4]);
        int stH = static_cast<int>(buffer[5]);

        int distance = distL + (distH << 8);
        int strength = stL + (stH << 8);

        int tempL = static_cast<int>(buffer[6]);
        int tempH = static_cast<int>(buffer[7]);
        int temperature = (tempL + (tempH << 8)) / 8 - 256;

        flush_input_buffer();
        distance =_medianFilter.apply(distance);

        _sensorData["distance"] = distance;
        _sensorData["strength"] = strength;
        _sensorData["temperature"] = temperature;

        return _sensorData;
    }
    
    flush_input_buffer();

    return _sensorData;  // Invalid data
    
}

