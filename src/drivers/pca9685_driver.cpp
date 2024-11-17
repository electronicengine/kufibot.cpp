
#include "pca9685_driver.h"
#include "../ui/main_window.h"

PCA9685Driver::PCA9685Driver(int address, bool debug) {
  i2c_fd = wiringPiI2CSetup(address);
  this->debug = debug;
  write(__MODE1, 0x00);
}

PCA9685Driver::~PCA9685Driver() {
  // No explicit resource deallocation needed with wiringPi
}

void PCA9685Driver::write(uint8_t reg, uint8_t value) {
  int result = wiringPiI2CWriteReg8(i2c_fd, reg, value);
  if (result<0){
    MainWindow::log("write error wiringPiI2CWriteReg8. reg: " + std::to_string(reg) + " val: " + std::to_string(value), LogLevel::LOG_ERROR);
  }
}

uint8_t PCA9685Driver::read(uint8_t reg) {
  uint8_t result = wiringPiI2CReadReg8(i2c_fd, reg);
  return result;
}

void PCA9685Driver::set_servo_pulse(int channel, int pulse) {
  // PWM frequency is assumed to be 50Hz, adjust if needed
  pulse = pulse * 4096 / 20000;
  set_pwm(channel, 0, pulse);
}

void PCA9685Driver::set_pwm_freq(int freq) {
  double prescaleval = 25000000.0; // 25MHz
  prescaleval = prescaleval / 4096.0; // 12-bit
  prescaleval = prescaleval / float(freq);
  prescaleval = prescaleval - 1.0;

  int prescale = int(std::floor(prescaleval + 0.5));

  uint8_t oldmode = read(__MODE1);
  uint8_t newmode = (oldmode & 0x7F) | 0x10; // sleep
  write(__MODE1, newmode); // go to sleep
  write(__PRESCALE, prescale);
  write(__MODE1, oldmode);
  delay(5);
  write(__MODE1, oldmode | 0x80);
}

void PCA9685Driver::set_pwm(int channel, int on, int off) {
  write(__LED0_ON_L + 4 * channel, on & 0xFF);
  write(__LED0_ON_H + 4 * channel, 0xFF & (on >> 8));
  write(__LED0_OFF_L + 4 * channel, off & 0xFF);
  write(__LED0_OFF_H + 4 * channel, 0xFF & (off >> 8));

}

void PCA9685Driver::set_duty_cycle(int channel, int duty) {
  set_pwm(channel, 0, int(duty * 4096 / 100));
}

void PCA9685Driver::set_level(int channel, int level) {
  if (level == 1) {
    set_pwm(channel, 0, 4095);
  } else {
    set_pwm(channel, 0, 0);
  }
}

