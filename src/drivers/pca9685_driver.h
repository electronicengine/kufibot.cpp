#ifndef PCA9685_DRIVER_H
#define PCA9685_DRIVER_H

#include <iostream>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <cmath>
#include "i2c_device.h"

#define PCA9685_ADDRESS 0x5f  // Default address of the PCA9685 chip

#define __SUBADR1 0x02
#define __SUBADR2 0x03
#define __SUBADR3 0x04
#define __MODE1 0x00
#define __PRESCALE 0xFE
#define __LED0_ON_L 0x06
#define __LED0_ON_H 0x07
#define __LED0_OFF_L 0x08
#define __LED0_OFF_H 0x09
#define __ALLLED_ON_L 0xFA
#define __ALLLED_ON_H 0xFB
#define __ALLLED_OFF_L 0xFC
#define __ALLLED_OFF_H 0xFD

class PCA9685Driver : public I2CDevice {
public:
    PCA9685Driver() = default;
    virtual ~PCA9685Driver();

    bool initPCA9685(int address);

    void setDutyCyclePulse(int channel, int pulse);
    void setPWMFrequency(int freq);
    void setDutyCyclePercent(int channel, int duty);
    void setChannelLevel(int channel, int level);

private:

    void setDutyCycle(int channel, int on, int off);

};

#endif