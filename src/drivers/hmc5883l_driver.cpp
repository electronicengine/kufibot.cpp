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


#include "hmc5883l_driver.h"
#include <thread>
#include <cmath>

HMC5883LDriver::HMC5883LDriver() {

}


HMC5883LDriver::~HMC5883LDriver() {

}


bool HMC5883LDriver::initHMC5883l() {

    bool ret = openDevice(HMC5883L_ADDRESS);
    if (!ret) {
        return false;
    }

    // Set to 8 samples @ 15Hz
    writeByte(HMC5883L_CONFIG_A, 0x70);

    // 1.3 gain LSb / Gauss 1090 (default)
    writeByte(HMC5883L_CONFIG_B, 0x20);

    // Continuous measurement mode
    writeByte(HMC5883L_MODE, 0x00);

    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 10ms delay for initialization

    return true;
}


Axis HMC5883LDriver::readHMC5883l() {
    Axis axis;

    axis.x = read_x();
    axis.y = read_y();
    axis.z = read_z();
    axis.t = compute_heading(axis.x, axis.y);

    return axis;
}



double HMC5883LDriver::compute_heading(int16_t x, int16_t y) {
    // Calculate heading in radians
    double heading_rad = atan2(y, x);

    // Adjust for declination angle (e.g. 0.22 for ~13 degrees)
    double declination_angle = 0.22;
    heading_rad += declination_angle;

    // Correct for when signs are reversed
    if (heading_rad < 0) {
        heading_rad += 2 * M_PI;
    }

    // Check for wrap due to addition of declination
    if (heading_rad > 2 * M_PI) {
        heading_rad -= 2 * M_PI;
    }

    // Convert radians to degrees
    double heading_deg = heading_rad * (180.0 / M_PI);

    return heading_deg;
}

int16_t HMC5883LDriver::read_x() {
    return readWord(HMC5883L_X_MSB);
}

int16_t HMC5883LDriver::read_y() {
    return readWord(HMC5883L_Y_MSB);
}

int16_t HMC5883LDriver::read_z() {
    return readWord(HMC5883L_Z_MSB);
}
