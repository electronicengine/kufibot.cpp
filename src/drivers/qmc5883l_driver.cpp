#include "qmc5883l_driver.h"
#include "../logger.h"

QMC5883LDriver::QMC5883LDriver(int address,
                                int output_data_rate, int output_range,
                                int oversampling_rate)
    : _address(address) {
    
    // Initialize WiringPi and I2C
    wiringPiSetup();
    _bus = wiringPiI2CSetup(address);
    if (_bus == -1) {
        Logger::error("Failed to initialize I2C bus.");
        exit(EXIT_FAILURE);
    }

    _mode_cont = (MODE_CONT | output_data_rate | output_range | oversampling_rate);
    _mode_stby = (MODE_STBY | ODR_10HZ | RNG_2G | OSR_64);
    mode_continuous();

//calibration matrix;
    _calibration[0][0] = 1.054714385985517;
    _calibration[0][1] = 0.0075999534062283075;
    _calibration[0][2] = 713.2896257128381;
    _calibration[1][0] = 0.007599953406228315;
    _calibration[1][1] = 1.001055650917697;
    _calibration[1][2] = 529.4850645299709;
    _calibration[2][0] = 0.0;
    _calibration[2][1] = 0.0;
    _calibration[2][2] = 1.0;

    _declination = 6.1; // istanbul 

}


QMC5883LDriver::~QMC5883LDriver() {
    mode_standby();
}

void QMC5883LDriver::mode_continuous() {
    write_byte(REG_CONTROL_2, 0b10000000); // Soft reset
    write_byte(REG_CONTROL_2, 0b00000000); // Disable interrupt
    write_byte(REG_RST_PERIOD, 0x01);      // Define SET/RESET period
    write_byte(REG_CONTROL_1, _mode_cont);   // Set operation mode
}

void QMC5883LDriver::mode_standby() {
    write_byte(REG_CONTROL_2, 0b10000000); // Soft reset
    write_byte(REG_CONTROL_2, 0b00000000); // Disable interrupt
    write_byte(REG_RST_PERIOD, 0x01);      // Define SET/RESET period
    write_byte(REG_CONTROL_1, _mode_stby);   // Set operation mode
}

void QMC5883LDriver::write_byte(uint8_t registry, uint8_t value) {
    wiringPiI2CWriteReg8(_bus, registry, value);
    delay(10); // Delay for stability
}

uint8_t QMC5883LDriver::read_byte(uint8_t registry) {
    return wiringPiI2CReadReg8(_bus, registry);
}

int16_t QMC5883LDriver::read_word(uint8_t registry) {
    uint8_t low = read_byte(registry);
    uint8_t high = read_byte(registry + 1);
    return (high << 8) | low;
}

int16_t QMC5883LDriver::read_word_2c(uint8_t registry) {
    int16_t val = read_word(registry);
    return val;
}

std::vector<int16_t> QMC5883LDriver::get_data() {
    std::vector<int16_t> data(4, 0); // x, y, z, temp
    int retries = 20;
    
    while (retries-- > 0) {
        uint8_t status = read_byte(REG_STATUS_1);
        if (status & 0b00000010) { // Overflow
            // MainWindow::log("Magnetic sensor overflow.", LogLevel::LOG_WARNING);
        }
        if (status & 0b00000100) { // Data skipped
            data[0] = read_word_2c(REG_XOUT_LSB);
            data[1] = read_word_2c(REG_YOUT_LSB);
            data[2] = read_word_2c(REG_ZOUT_LSB);
            continue;
        }
        if (status & 0b00000001) { // Data ready
            data[0] = read_word_2c(REG_XOUT_LSB);
            data[1] = read_word_2c(REG_YOUT_LSB);
            data[2] = read_word_2c(REG_ZOUT_LSB);
            data[3] = read_word_2c(REG_TOUT_LSB);
            break;
        }
        delay(10); // Wait for data ready
    }
    return data; // Return data with x, y, z as 0 if timeout
}

std::vector<int16_t> QMC5883LDriver::get_magnet() {

    // Get raw magnetometer data
    std::vector<int16_t> raw_data = get_data();
    int16_t x = raw_data[0];
    int16_t y = raw_data[1];

    double x1, y1;

    // Check if x or y is invalid
    if (x == 0 && y == 0) { // Assuming 0 indicates invalid data
        x1 = x;
        y1 = y;
    } else {
        // Apply calibration
        x1 = x * _calibration[0][0] + y * _calibration[0][1] + _calibration[0][2];
        y1 = x * _calibration[1][0] + y * _calibration[1][1] + _calibration[1][2];
    }

    return {static_cast<int16_t>(x1), static_cast<int16_t>(y1)};
}


void QMC5883LDriver::apply_calibration(const std::vector<int16_t>& raw, double& x_cal, double& y_cal, double& z_cal) {
    // Apply calibration: (raw - offset) / scale
    x_cal = (raw[0] - offsets[0]) / scales[0];
    y_cal = (raw[1] - offsets[1]) / scales[1];
    z_cal = (raw[2] - offsets[2]) / scales[2];
}

double QMC5883LDriver::get_bearing() {
    auto magnet = get_magnet();
    double x = magnet[0];
    double y = magnet[1];

    if (std::isnan(x) || std::isnan(y)) {
        return NAN; // Return NaN if invalid data
    }

    double bearing = std::atan2(y, x) * (180.0 / M_PI); // Convert radians to degrees
    if (bearing < 0) {
        bearing += 360.0;
    }
    bearing += _declination; // Adjust for declination
    if (bearing < 0.0) {
        bearing += 360.0;
    } else if (bearing >= 360.0) {
        bearing -= 360.0;
    }
    return bearing;
}
