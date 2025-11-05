
#ifndef INA219_DRIVER_H
#define INA219_DRIVER_H

#include <stdint.h>
#include "i2c_device.h"
#include "driver_data.h"

#include "i2c_device.h"
#include <cstdint>

class INA219Driver : public I2CDevice {
public:
    // I2C Addresses
    static constexpr uint8_t INA219_I2C_ADDRESS1 = 0x40;
    static constexpr uint8_t INA219_I2C_ADDRESS2 = 0x41;
    static constexpr uint8_t INA219_I2C_ADDRESS3 = 0x44;
    static constexpr uint8_t INA219_I2C_ADDRESS4 = 0x45;

    // Bus Voltage Range
    enum BusVoltageRange {
        BUS_VOL_RANGE_16V = 0,
        BUS_VOL_RANGE_32V = 1
    };

    // PGA Gain
    enum PGABits {
        PGA_BITS_1 = 0,
        PGA_BITS_2 = 1,
        PGA_BITS_4 = 2,
        PGA_BITS_8 = 3
    };

    // ADC Resolution
    enum ADCBits {
        ADC_BITS_9  = 0,
        ADC_BITS_10 = 1,
        ADC_BITS_11 = 2,
        ADC_BITS_12 = 3
    };

    // ADC Samples
    enum ADCSample {
        ADC_SAMPLE_1   = 0,
        ADC_SAMPLE_2   = 1,
        ADC_SAMPLE_4   = 2,
        ADC_SAMPLE_8   = 3,
        ADC_SAMPLE_16  = 4,
        ADC_SAMPLE_32  = 5,
        ADC_SAMPLE_64  = 6,
        ADC_SAMPLE_128 = 7
    };

    // Operating Mode
    enum Mode {
        POWER_DOWN                = 0,
        SHUNT_VOL_TRIG           = 1,
        BUS_VOL_TRIG             = 2,
        SHUNT_AND_BUS_VOL_TRIG   = 3,
        ADC_OFF                   = 4,
        SHUNT_VOL_CON            = 5,
        BUS_VOL_CON              = 6,
        SHUNT_AND_BUS_VOL_CON    = 7
    };

    INA219Driver();
    ~INA219Driver();

    // Initialize the sensor
    bool initINA219(int address = INA219_I2C_ADDRESS3);
    INA219Data readINA219();

    // Reset the device
    void reset();

    // Read measurements
    float getBusVoltageV();
    float getShuntVoltageMV();
    float getCurrentMA();
    float getPowerMW();

    // Configuration
    void setBusRange(BusVoltageRange range);
    void setPGA(PGABits bits);
    void setBusADC(ADCBits bits, ADCSample sample);
    void setShuntADC(ADCBits bits, ADCSample sample);
    void setMode(Mode mode);
    void setCalibration(float shuntResistor_ohms, float maxCurrent_A);

private:
    float _currentLSB;
    // Register addresses
    static constexpr uint8_t REG_CONFIG       = 0x00;
    static constexpr uint8_t REG_SHUNTVOLTAGE = 0x01;
    static constexpr uint8_t REG_BUSVOLTAGE   = 0x02;
    static constexpr uint8_t REG_POWER        = 0x03;
    static constexpr uint8_t REG_CURRENT      = 0x04;
    static constexpr uint8_t REG_CALIBRATION  = 0x05;

    // Configuration values
    static constexpr uint16_t CONFIG_RESET = 0x8000;

    // Member variables
    uint16_t _calValue;

    // Helper functions
    void writeRegister(uint8_t reg, uint16_t value);
    int16_t readRegister(uint8_t reg);
    bool scan();
};

#endif
