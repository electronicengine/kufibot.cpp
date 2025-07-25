#include "ina219_driver.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <bitset>
#include <math.h>
#include "../logger.h"

INA219Driver::INA219Driver(float shunt_resistance, float max_expected_amps)
{
	init_i2c(__ADDRESS);

	_shunt_ohms = shunt_resistance;
	_max_expected_amps = max_expected_amps;
	_min_device_current_lsb = __CALIBRATION_FACTOR / (_shunt_ohms * __MAX_CALIBRATION_VALUE);
}
INA219Driver::INA219Driver(float shunt_resistance, float max_expected_amps, uint8_t address)
{
	init_i2c(address);

	_shunt_ohms = shunt_resistance;
	_max_expected_amps = max_expected_amps;
	_min_device_current_lsb = __CALIBRATION_FACTOR / (_shunt_ohms * __MAX_CALIBRATION_VALUE);
}

INA219Driver::~INA219Driver()
{
	close(_file_descriptor);
}


void
INA219Driver::init_i2c(uint8_t address)
{
	char *filename = (char*)"/dev/i2c-1";
	if ((_file_descriptor = open(filename, O_RDWR)) < 0)
	{
		ERROR("Failed to open the i2c bus");
	}
	if (ioctl(_file_descriptor, I2C_SLAVE, address) < 0)
	{
		ERROR("Failed to acquire bus access and/or talk to slave device: ");
	}
}

uint16_t
INA219Driver::read_register(uint8_t register_address)
{
	uint8_t buf[3];
	buf[0] = register_address;
	if (write(_file_descriptor, buf, 1) != 1) {
		ERROR("Failed to set register");
	}
	usleep(1000);
	if (read(_file_descriptor, buf, 2) != 2) {
		ERROR("Failed to read register value");
	}
	return (buf[0] << 8) | buf[1];
}
void
INA219Driver::write_register(uint8_t register_address, uint16_t register_value)
{
	uint8_t buf[3];
	buf[0] = register_address;
	buf[1] = register_value >> 8;
	buf[2] = register_value & 0xFF;
	
	if (write(_file_descriptor, buf, 3) != 3)
	{
		ERROR("Failed to write to the i2c bus");
	}
}

void
INA219Driver::configure(int voltage_range, int gain, int bus_adc, int shunt_adc)
{
	reset();
	
	int len = sizeof(__BUS_RANGE) / sizeof(__BUS_RANGE[0]);
	if (voltage_range > len-1) {
		ERROR("Invalid voltage range, must be one of: RANGE_16V, RANGE_32");
	}
	_voltage_range = voltage_range;
	_gain = gain;

	calibrate(__GAIN_VOLTS[gain], _max_expected_amps);
	uint16_t calibration = (voltage_range << __BRNG | _gain << __PG0 | bus_adc << __BADC1 | shunt_adc << __SADC1 | __CONT_SH_BUS);
	write_register(__REG_CONFIG, calibration);
}

void
INA219Driver::calibrate(float shunt_volts_max, float max_expected_amps)
{
	float max_possible_amps = shunt_volts_max / _shunt_ohms;
	_current_lsb = determine_current_lsb(max_expected_amps, max_possible_amps);
	_power_lsb = _current_lsb * 20.0;
	uint16_t calibration = (uint16_t) trunc(__CALIBRATION_FACTOR / (_current_lsb * _shunt_ohms));
	write_register(__REG_CALIBRATION, calibration);
}
float
INA219Driver::determine_current_lsb(float max_expected_amps, float max_possible_amps)
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
void
INA219Driver::sleep()
{
	uint16_t config = read_register(__REG_CONFIG);
	write_register(__REG_CONFIG, config & 0xFFF8);
}
void
INA219Driver::wake()
{
	uint16_t config = read_register(__REG_CONFIG);
	write_register(__REG_CONFIG, config | 0x0007);
	// 40us delay to recover from powerdown (p14 of spec)
	usleep(40);
}
void
INA219Driver::reset()
{
	write_register(__REG_CONFIG, __RST);
}
float
INA219Driver::voltage()
{
	uint16_t value = read_register(__REG_BUSVOLTAGE) >> 3;
    return float(value) * __BUS_MILLIVOLTS_LSB / 1000.0;
}
float
INA219Driver::shunt_voltage()
{
	uint16_t shunt_voltage = read_register(__REG_SHUNTVOLTAGE);
	return __SHUNT_MILLIVOLTS_LSB * (int16_t)shunt_voltage;
}
float
INA219Driver::supply_voltage()
{
	return voltage() + (shunt_voltage() / 1000.0);
}
float
INA219Driver::current()
{
	uint16_t current_raw = read_register(__REG_CURRENT);
	int16_t current = (int16_t)current_raw;
	return  current * _current_lsb * 1000.0;
}
float
INA219Driver::power()
{
	uint16_t power_raw = read_register(__REG_POWER);
	int16_t power = (int16_t)power_raw;
	return power * _power_lsb * 1000.0;
}