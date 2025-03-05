#ifndef QMC5883L_DRIVER_H
#define QMC5883L_DRIVER_H

#include <iostream>
#include <iomanip>
#include <math.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <vector>
#include <array>

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

class QMC5883LDriver {
public:
    QMC5883LDriver(int address = DFLT_ADDRESS,
                   int output_data_rate = ODR_10HZ, int output_range = RNG_2G,
                   int oversampling_rate = OSR_512);

    ~QMC5883LDriver();

    void mode_continuous();
    void mode_standby();
    void write_byte(uint8_t registry, uint8_t value);
    uint8_t read_byte(uint8_t registry);
    int16_t read_word(uint8_t registry);
    int16_t read_word_2c(uint8_t registry);
    std::vector<int16_t> get_data();
    double get_bearing();
    std::vector<int16_t> get_magnet();
private:
    int _bus;
    int _address;
    int _mode_cont;
    int _mode_stby;

    double _declination;                          
    std::array<std::array<double, 3>, 3> _calibration; // Calibration matrix
   // Offsets and scales for calibration
    const std::vector<double> offsets = {3096.95455451, -7452.84550654, 840.2791695};
    const std::vector<double> scales = {16639.36998797, 18514.63340064, 16972.20317855};

    // Apply calibration to raw data
    void apply_calibration(const std::vector<int16_t>& raw, double& x_cal, double& y_cal, double& z_cal);

};

#endif
