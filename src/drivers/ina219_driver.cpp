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

INA219Driver::INA219Driver()
{
	_shunt_ohms = SHUNT_OHMS;
	_max_expected_amps = MAX_EXPECTED_AMPS;
	_min_device_current_lsb = __CALIBRATION_FACTOR / (_shunt_ohms * __MAX_CALIBRATION_VALUE);

}

INA219Driver::~INA219Driver() { }

bool INA219Driver::initINA219()
{
    bool ret = openDevice(__ADDRESS);
    if (ret) {
        _configure(RANGE_16V, GAIN_8_320MV, ADC_12BIT, ADC_12BIT);
    }
}
INA219Data INA219Driver::readINA219() {

    INA219Data data;
    data.voltage = _voltage();
    data.shunt_voltage = _shuntVoltage();
    data.supply_voltage = data.voltage + (data.shunt_voltage / 1000.0);
    data.current = _current();
    data.power = _power();

    return data;
}


void INA219Driver::_configure(int voltage_range, int gain, int bus_adc, int shunt_adc)
{
	_reset();
	
	int len = sizeof(__BUS_RANGE) / sizeof(__BUS_RANGE[0]);
	if (voltage_range > len-1) {
		ERROR("Invalid voltage range, must be one of: RANGE_16V, RANGE_32");
	}
	_voltage_range = voltage_range;
	_gain = gain;

	_calibrate(__GAIN_VOLTS[gain], _max_expected_amps);
	uint16_t calibration = (voltage_range << __BRNG | _gain << __PG0 | bus_adc << __BADC1 | shunt_adc << __SADC1 | __CONT_SH_BUS);
	writeWord(__REG_CONFIG, calibration);
}

void INA219Driver::_calibrate(float shunt_volts_max, float max_expected_amps)
{
	float max_possible_amps = shunt_volts_max / _shunt_ohms;
	_current_lsb = _determineCurrentLSB(max_expected_amps, max_possible_amps);
	_power_lsb = _current_lsb * 20.0;
	uint16_t calibration = (uint16_t) trunc(__CALIBRATION_FACTOR / (_current_lsb * _shunt_ohms));
	writeWord(__REG_CALIBRATION, calibration);
}

float INA219Driver::_determineCurrentLSB(float max_expected_amps, float max_possible_amps)
{
	float current_lsb;

	float nearest = roundf(max_possible_amps * 1000.0) / 1000.0;
	if (max_expected_amps > nearest) {
		ERROR("Expected current {} A is greater than max possible current {} A", max_expected_amps, max_possible_amps);
	}

	if (max_expected_amps < max_possible_amps) {
		current_lsb = max_expected_amps / __CURRENT_LSB_FACTOR;
	} else {
		current_lsb = max_possible_amps / __CURRENT_LSB_FACTOR;
	}
	
	if (current_lsb < _min_device_current_lsb) {
		current_lsb = _min_device_current_lsb;
	}
	return current_lsb;
}

void INA219Driver::_sleep()
{
	uint16_t config = readWord(__REG_CONFIG);
	writeWord(__REG_CONFIG, config & 0xFFF8);
}

void INA219Driver::_wake()
{
	uint16_t config = readWord(__REG_CONFIG);
	writeWord(__REG_CONFIG, config | 0x0007);
	// 40us delay to recover from powerdown (p14 of spec)
	usleep(40);
}

void INA219Driver::_reset()
{
	writeWord(__REG_CONFIG, __RST);
}


float INA219Driver::_voltage()
{
	uint16_t value = readWord(__REG_BUSVOLTAGE) >> 3;
    return float(value) * __BUS_MILLIVOLTS_LSB / 1000.0;
}


float INA219Driver::_shuntVoltage()
{
	uint16_t shunt_voltage = readWord(__REG_SHUNTVOLTAGE);
	return __SHUNT_MILLIVOLTS_LSB * (int16_t)shunt_voltage;
}

float INA219Driver::_current()
{
	uint16_t current_raw = readWord(__REG_CURRENT);
	int16_t current = (int16_t)current_raw;
	return  current * _current_lsb * 1000.0;
}

float INA219Driver::_power()
{
	uint16_t power_raw = readWord(__REG_POWER);
	int16_t power = (int16_t)power_raw;
	return power * _power_lsb * 1000.0;
}