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

#include "i2c_device.h"
#include "../logger.h"

std::mutex I2CDevice::_mutex;


I2CDevice::I2CDevice() {

}

I2CDevice::~I2CDevice() {

}

bool I2CDevice::isReady() const {
    return _i2cFd != -1;
}

bool I2CDevice::openDevice(int address) {
    std::lock_guard<std::mutex> lockGuard(_mutex);

    _i2cFd = wiringPiI2CSetup(address);
    if (_i2cFd == -1) {
        ERROR("I2C setup failed for address {}", address);
        return false;
    }

    return true;
}

bool I2CDevice::writeByte(uint8_t reg, uint8_t value) {
    std::lock_guard<std::mutex> lockGuard(_mutex);
    return wiringPiI2CWriteReg8(_i2cFd, reg, value) >= 0;
}

int I2CDevice::readByte(uint8_t reg) {
    std::lock_guard<std::mutex> lockGuard(_mutex);
    return wiringPiI2CReadReg8(_i2cFd, reg);
}

bool I2CDevice::writeWord(uint8_t reg, uint16_t value) {
    std::lock_guard<std::mutex> lockGuard(_mutex);
    return wiringPiI2CWriteReg16(_i2cFd, reg, value);
}

int I2CDevice::readWord(uint8_t reg) {
    std::lock_guard<std::mutex> lockGuard(_mutex);
    return wiringPiI2CReadReg16(_i2cFd, reg);
}
