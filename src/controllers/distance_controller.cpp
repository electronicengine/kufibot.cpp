/*
* This file is part of Kufibot.
 *
 * Kufibot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kufibot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kufibot. If not, see <https://www.gnu.org/licenses/>.
 */

#include "distance_controller.h"
#include "../logger.h"

DistanceController* DistanceController::_instance = nullptr;

bool DistanceController::is_active()
{
    if (!uart.is_open()) {
        return false;
    }else{
        return true;
    }
}

DistanceController *DistanceController::get_instance()
{
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


DistanceData DistanceController::get_distance() {

    if (!_enable.load()) {
        WARNING("DistanceController is disabled");
        return DistanceData{};
    }

    DistanceData data;
    std::vector<uint8_t> buffer(9);
    boost::system::error_code ec;
    
    // Read 9 bytes from the serial port
    size_t bytes_read = boost::asio::read(uart, boost::asio::buffer(buffer), ec);

    if (ec || bytes_read != 9) {
        ERROR("Failed to read data: {}" +  ec.message());
        return data;  // Error case
    }

    if (buffer[0] == 'Y' && buffer[1] == 'Y') {
        int distL = static_cast<int>(buffer[2]);
        int distH = static_cast<int>(buffer[3]);
        int stL = static_cast<int>(buffer[4]);
        int stH = static_cast<int>(buffer[5]);

        data.distance = distL + (distH << 8);
        data.strength = stL + (stH << 8);



        int tempL = static_cast<int>(buffer[6]);
        int tempH = static_cast<int>(buffer[7]);
        data.temperature = (tempL + (tempH << 8)) / 8 - 256;

        flush_input_buffer();
        //data.distance =_medianFilter.apply(data.distance);

        return data;
    }
    
    flush_input_buffer();

    return data;  // Invalid data
    
}

