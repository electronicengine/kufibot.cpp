#ifndef QMC5883L_DRIVER_H
#define QMC5883L_DRIVER_H


#include "wiringPi.h"
#include "wiringPiI2C.h"
#include "driver_data.h"
#include <vector>
#include <array>
#include "i2c_device.h"

#define DFLT_BUS 1
#define DFLT_ADDRESS 0x0D

// Register Definitions
#define REG_XOUT_LSB 0x00
#define REG_XOUT_MSB 0x01
#define REG_YOUT_LSB 0x02
#define REG_YOUT_MSB 0x03
#define REG_ZOUT_LSB 0x04
#define REG_ZOUT_MSB 0x05
#define REG_STATUS_1 0x06
#define REG_TOUT_LSB 0x07
#define REG_TOUT_MSB 0x08
#define REG_CONTROL_1 0x09
#define REG_CONTROL_2 0x0A
#define REG_RST_PERIOD 0x0B
#define REG_CHIP_ID 0x0D

// Control Register 1 Modes
#define MODE_STBY 0b00000000
#define MODE_CONT 0b00000001
#define ODR_10HZ 0b00000000
#define ODR_50HZ 0b00000100
#define ODR_100HZ 0b00001000
#define ODR_200HZ 0b00001100
#define RNG_2G 0b00000000
#define RNG_8G 0b00010000
#define OSR_512 0b00000000
#define OSR_256 0b01000000
#define OSR_128 0b10000000
#define OSR_64 0b11000000

class QMC5883LDriver : public I2CDevice {
public:
    QMC5883LDriver();

    virtual ~QMC5883LDriver();


    bool initQMC5883l();
    Axis readQMC5883l();


private:
    int _bus;
    int _address;
    int _mode_cont;
    int _mode_stby;


    int _outputDataRate;
    int _outputRange;
    int _overSamplingRate;
    double _declination;

    std::array<std::array<double, 3>, 3> _calibration; // Calibration matrix
   // Offsets and scales for calibration
    const std::vector<double> offsets = {3096.95455451, -7452.84550654, 840.2791695};
    const std::vector<double> scales = {16639.36998797, 18514.63340064, 16972.20317855};

    // Apply calibration to raw data
    void _applyCalibration(const std::vector<int16_t>& raw, double& x_cal, double& y_cal, double& z_cal);
    void _modeContinuous();
    void _modeStandby();
    double _getBearing();
    Axis _getMagnet();
};

#endif