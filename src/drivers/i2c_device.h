//
// Created by ybulb on 10/31/2025.
//

#ifndef I2C_DEVICE_H
#define I2C_DEVICE_H


#include <wiringPiI2C.h>
#include <cstdint>
#include <string>
#include <mutex>
#include "driver.h"



class I2CDevice : public Driver {


public:
    explicit I2CDevice(const std::string& name);
    ~I2CDevice() override;

    void shutdown() override;
    bool isReady() const noexcept override;
    bool openDevice(int address);
    bool writeByte(uint8_t reg, uint8_t value);
    int  readByte(uint8_t reg);
    bool writeWord(uint8_t reg, uint16_t value);
    int  readWord(uint8_t reg);

private:
    static std::mutex _mutex;
    int _i2cFd;
    int _address;

protected:
    void closeDevice();

};



#endif //I2C_DEVICE_H
