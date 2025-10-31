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

#include "qmc5883l_driver.h"
#include <cmath>

QMC5883LDriver::QMC5883LDriver() {

    _address = DFLT_ADDRESS;
    _outputDataRate = ODR_10HZ;
    _outputRange = RNG_2G;
    _overSamplingRate = OSR_512;

}


QMC5883LDriver::~QMC5883LDriver() {
    _modeStandby();
}

bool QMC5883LDriver::initQMC5883l()
{
        // Initialize WiringPi and I2C
    bool ret = openDevice(_address);

    _mode_cont = (MODE_CONT | _outputDataRate | _outputRange | _overSamplingRate);
    _mode_stby = (MODE_STBY | ODR_10HZ | RNG_2G | OSR_64);
    _modeContinuous();

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

    return ret;
}

void QMC5883LDriver::_modeContinuous() {
    writeByte(REG_CONTROL_2, 0b10000000); // Soft reset
    writeByte(REG_CONTROL_2, 0b00000000); // Disable interrupt
    writeByte(REG_RST_PERIOD, 0x01);      // Define SET/RESET period
    writeByte(REG_CONTROL_1, _mode_cont);   // Set operation mode
}

void QMC5883LDriver::_modeStandby() {
    writeByte(REG_CONTROL_2, 0b10000000); // Soft reset
    writeByte(REG_CONTROL_2, 0b00000000); // Disable interrupt
    writeByte(REG_RST_PERIOD, 0x01);      // Define SET/RESET period
    writeByte(REG_CONTROL_1, _mode_stby);   // Set operation mode
}

Axis QMC5883LDriver::readQMC5883l() {
    Axis data;

    uint8_t status = readByte(REG_STATUS_1);
    if (status & 0b00000010) { // Overflow
        // MainWindow::log("Magnetic sensor overflow.", LogLevel::LOG_WARNING);
    }
    if (status & 0b00000100 || status & 0b00000001) { // Data skipped
        data.x = readWord(REG_XOUT_LSB);
        data.y = readWord(REG_YOUT_LSB);
        data.z = readWord(REG_ZOUT_LSB);
        //data[3] = read_word_2c(REG_TOUT_LSB);

    }

    data.x = data.x * _calibration[0][0] + data.y * _calibration[0][1] + _calibration[0][2];
    data.y = data.x * _calibration[1][0] + data.y * _calibration[1][1] + _calibration[1][2];

    return data; // Return data with x, y, z as 0 if timeout
}

Axis QMC5883LDriver::_getMagnet() {

    // Get raw magnetometer data
    Axis data = readQMC5883l();

    // Check if x or y is invalid
    if (data.x == 0 && data.y == 0) { // Assuming 0 indicates invalid data

    } else {
        // Apply calibration
        data.x = data.x * _calibration[0][0] + data.y * _calibration[0][1] + _calibration[0][2];
        data.y = data.x * _calibration[1][0] + data.y * _calibration[1][1] + _calibration[1][2];
    }

    return data;
}


void QMC5883LDriver::_applyCalibration(const std::vector<int16_t>& raw, double& x_cal, double& y_cal, double& z_cal) {
    // Apply calibration: (raw - offset) / scale
    x_cal = (raw[0] - offsets[0]) / scales[0];
    y_cal = (raw[1] - offsets[1]) / scales[1];
    z_cal = (raw[2] - offsets[2]) / scales[2];
}

double QMC5883LDriver::_getBearing() {
    Axis data = _getMagnet();

    if (std::isnan(data.x) || std::isnan(data.y)) {
        return -1; // Return NaN if invalid data
    }

    double bearing = std::atan2(data.y, data.x) * (180.0 / M_PI); // Convert radians to degrees
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