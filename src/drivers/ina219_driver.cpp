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

#include "ina219_driver.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <bitset>
#include <math.h>
#include "../logger.h"

#include <thread>
#include <chrono>

INA219Driver::INA219Driver() : I2CDevice() {
}

INA219Driver::~INA219Driver() {
}

bool INA219Driver::initINA219(int address) {
    if (!openDevice(address)) {
        ERROR("Failed to open INA219 at address 0x{:02X}", address);
        return false;
    }

    if (!scan()) {
        ERROR("INA219 not found at address 0x{:02X}", address);
        return false;
    }

    setBusRange(BUS_VOL_RANGE_32V);
    setPGA(PGA_BITS_8);
    setBusADC(ADC_BITS_12, ADC_SAMPLE_8);
    setShuntADC(ADC_BITS_12, ADC_SAMPLE_8);
    setMode(SHUNT_AND_BUS_VOL_CON);
    setCalibration(0.1, 3.2);

    INFO("INA219 initialized successfully at address 0x{:02X}", address);
    return true;
}

INA219Data INA219Driver::readINA219() {
    INA219Data data;
    data.voltage = getBusVoltageV();
    data.current = getCurrentMA();
    data.power = getPowerMW();
    return data;
}

void INA219Driver::reset() {
    writeRegister(REG_CONFIG, CONFIG_RESET);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    INFO("INA219 reset");
}

float INA219Driver::getBusVoltageV() {
    int16_t value = readRegister(REG_BUSVOLTAGE);
    return static_cast<float>(value >> 1) * 0.001f;
}

float INA219Driver::getShuntVoltageMV() {
    int16_t value = readRegister(REG_SHUNTVOLTAGE);
    return static_cast<float>(value);
}

float INA219Driver::getCurrentMA() {
    int16_t value = readRegister(REG_CURRENT);
    return static_cast<float>(value) * _currentLSB;
}

float INA219Driver::getPowerMW() {
    int16_t value = readRegister(REG_POWER);
    return static_cast<float>(value) * 20.0f;
}

void INA219Driver::setBusRange(BusVoltageRange range) {
    uint16_t conf = readRegister(REG_CONFIG);
    conf &= ~(0x01 << 13);
    conf |= static_cast<uint16_t>(range) << 13;
    writeRegister(REG_CONFIG, conf);
}

void INA219Driver::setPGA(PGABits bits) {
    uint16_t conf = readRegister(REG_CONFIG);
    conf &= ~(0x03 << 11);
    conf |= static_cast<uint16_t>(bits) << 11;
    writeRegister(REG_CONFIG, conf);
}

void INA219Driver::setBusADC(ADCBits bits, ADCSample sample) {
    uint16_t value = 0;

    if (bits < ADC_BITS_12 && sample > ADC_SAMPLE_1) {
        WARNING("Invalid ADC configuration ");
        return;
    }

    if (bits < ADC_BITS_12) {
        value = static_cast<uint16_t>(bits);
    } else {
        value = 0x80 | static_cast<uint16_t>(sample);
    }

    uint16_t conf = readRegister(REG_CONFIG);
    conf &= ~(0x0F << 7);
    conf |= value << 7;
    writeRegister(REG_CONFIG, conf);
}

void INA219Driver::setShuntADC(ADCBits bits, ADCSample sample) {
    uint16_t value = 0;

    if (bits < ADC_BITS_12 && sample > ADC_SAMPLE_1) {
        WARNING("Invalid ADC configuration:");
        return;
    }

    if (bits < ADC_BITS_12) {
        value = static_cast<uint16_t>(bits);
    } else {
        value = 0x80 | static_cast<uint16_t>(sample);
    }

    uint16_t conf = readRegister(REG_CONFIG);
    conf &= ~(0x0F << 3);
    conf |= value << 3;
    writeRegister(REG_CONFIG, conf);
}

void INA219Driver::setMode(Mode mode) {
    uint16_t conf = readRegister(REG_CONFIG);
    conf &= ~0x07;
    conf |= static_cast<uint16_t>(mode);
    writeRegister(REG_CONFIG, conf);
}

void INA219Driver::setCalibration(float shuntResistor_ohms, float maxCurrent_A) {
    // Current_LSB = maxCurrent_A / 32768
    float currentLSB = maxCurrent_A / 32768.0f;

    // Cal = 0.04096 / (Current_LSB * Shunt Resistor)
    _calValue = static_cast<uint16_t>(0.04096f / (currentLSB * shuntResistor_ohms));
    _calValue &= 0xFFFE; // Make sure LSB is 0

    writeRegister(REG_CALIBRATION, _calValue);

    // Current LSB'yi sakla
    _currentLSB = currentLSB * 1000.0f; // mA cinsinden
}


void INA219Driver::writeRegister(uint8_t reg, uint16_t value) {
    // INA219 uses big-endian byte order, but wiringPi expects little-endian
    // We need to swap bytes for proper communication
    uint16_t swappedValue = ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);

    if (!writeWord(reg, swappedValue)) {
    }
}

int16_t INA219Driver::readRegister(uint8_t reg) {
    int result = readWord(reg);

    if (result < 0) {
        ERROR("Failed to read from register 0x{:02X}", reg);
        return 0;
    }

    // Swap bytes back from little-endian to big-endian
    uint16_t value = static_cast<uint16_t>(result);
    uint16_t swappedValue = ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);

    // Handle two's complement for negative values
    if (swappedValue & 0x8000) {
        return static_cast<int16_t>(swappedValue - 0x10000);
    }

    return static_cast<int16_t>(swappedValue);
}

bool INA219Driver::scan() {
    // Try to read from CONFIG register to verify device presence
    int result = readWord(REG_CONFIG);

    if (result < 0) {
        ERROR("INA219 scan failed - device not responding");
        return false;
    }

    return true;
}