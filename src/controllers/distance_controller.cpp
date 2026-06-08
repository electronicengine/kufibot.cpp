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
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>

DistanceController* DistanceController::_instance = nullptr;

bool DistanceController::is_active()
{
    return isReady();
}

DistanceController *DistanceController::get_instance()
{
    if (_instance == nullptr) {
        _instance = new DistanceController();
    }
    return _instance;
}

DistanceController::DistanceController()
    : Controller("DistanceController") {
    if (!DistanceController::initialize()) {
        WARNING("{} failed to initialize", getName());
    }
}

bool DistanceController::initialize()
{
    if (uart_fd >= 0) {
        _initialized.store(true);
        return true;
    }

    uart_fd = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_SYNC);
    if (uart_fd < 0) {
        ERROR("Failed to open UART device");
        _initialized.store(false);
        return false;
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(uart_fd, &tty) != 0) {
        ERROR("Error from tcgetattr");
        close(uart_fd);
        uart_fd = -1;
        _initialized.store(false);
        return false;
    }


    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo, no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 9;            // read blocks until 9 bytes received
    tty.c_cc[VTIME] = 1;            // 0.1 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls, enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag &= ~CSTOPB;                 // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;                // no flow control

    if (tcsetattr(uart_fd, TCSANOW, &tty) != 0) {
        ERROR("Error from tcsetattr");
        close(uart_fd);
        uart_fd = -1;
        _initialized.store(false);
        return false;
    }

    _initialized.store(true);
    return true;
}

DistanceController::~DistanceController(){
    DistanceController::shutdown();
}

void DistanceController::shutdown() {
    if (uart_fd >= 0) {
        close(uart_fd);
        uart_fd = -1;
    }
    _initialized.store(false);
}

bool DistanceController::isReady() const noexcept {
    return _initialized.load() && uart_fd >= 0;
}

void DistanceController::flush_input_buffer() {
    if (uart_fd >= 0)
        tcflush(uart_fd, TCIFLUSH);
}

DistanceData DistanceController::get_distance() {

    if (!isEnabled()) {
        WARNING("DistanceController is disabled");
        return DistanceData{};
    }

    if (!isReady()) {
        ERROR("UART device couldnt opened!");
        return DistanceData{};
    }

    uint8_t buffer[9]{};
    struct timeval timeout;
    timeout.tv_sec = 1; // 1 second timeout
    timeout.tv_usec = 0;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(uart_fd, &readfds);

    if (select(uart_fd + 1, &readfds, NULL, NULL, &timeout) > 0) {
        const ssize_t bytes_read = read(uart_fd, buffer, sizeof(buffer));
        if (bytes_read < static_cast<ssize_t>(sizeof(buffer))) {
            ERROR("Incomplete UART frame read: {} bytes", bytes_read);
            flush_input_buffer();
            return DistanceData{};
        }
    } else {
        ERROR("Read timeout");
        return DistanceData{}; // Return empty data on timeout
    }


    if (buffer[0] == 'Y' && buffer[1] == 'Y') {
        int distL = static_cast<int>(buffer[2]);
        int distH = static_cast<int>(buffer[3]);
        int stL = static_cast<int>(buffer[4]);
        int stH = static_cast<int>(buffer[5]);

        int tempL = static_cast<int>(buffer[6]);
        int tempH = static_cast<int>(buffer[7]);

        flush_input_buffer();
        //data.distance =_medianFilter.apply(data.distance);

        return DistanceData{
            distL + (distH << 8),
            stL + (stH << 8),
            (tempL + (tempH << 8)) / 8 - 256
        };
    }

    flush_input_buffer();
    return DistanceData{};  // Invalid data
}