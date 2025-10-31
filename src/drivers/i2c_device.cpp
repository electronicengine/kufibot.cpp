//
// Created by ybulb on 10/31/2025.
//

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
    return wiringPiI2CReadReg8(_i2cFd, reg);
}

int I2CDevice::readWord(uint8_t reg) {
    std::lock_guard<std::mutex> lockGuard(_mutex);
    return wiringPiI2CReadReg16(_i2cFd, reg);
}
