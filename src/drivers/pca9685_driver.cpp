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

#include "pca9685_driver.h"
#include "../logger.h"

PCA9685Driver::~PCA9685Driver() {
    // No explicit resource deallocation needed with wiringPi
}
bool PCA9685Driver::initPCA9685(int address) {
    bool ret = openDevice(address);
    if (ret) {
        writeByte(__MODE1, 0x00);
    }
    return ret;
}

void PCA9685Driver::setDutyCyclePulse(int channel, int pulse) {
    // PWM frequency is assumed to be 50Hz, adjust if needed
    //  pulse μs (500–2500)
    pulse = pulse * 4096 / 20000;
    setDutyCycle(channel, 0, pulse);
}

void PCA9685Driver::setPWMFrequency(int freq) {
  double prescaleval = 25000000.0; // 25MHz
  prescaleval = prescaleval / 4096.0; // 12-bit
  prescaleval = prescaleval / float(freq);
  prescaleval = prescaleval - 1.0;

  int prescale = int(std::floor(prescaleval + 0.5));

  uint8_t oldmode = readByte(__MODE1);
  uint8_t newmode = (oldmode & 0x7F) | 0x10; // sleep
  writeByte(__MODE1, newmode); // go to sleep
  writeByte(__PRESCALE, prescale);
  writeByte(__MODE1, oldmode);
  delay(5);
  writeByte(__MODE1, oldmode | 0x80);
}

void PCA9685Driver::setDutyCycle(int channel, int on, int off) {
    writeByte(__LED0_ON_L + 4 * channel, on & 0xFF);
    writeByte(__LED0_ON_H + 4 * channel, 0xFF & (on >> 8));
    writeByte(__LED0_OFF_L + 4 * channel, off & 0xFF);
    writeByte(__LED0_OFF_H + 4 * channel, 0xFF & (off >> 8));
}

void PCA9685Driver::setDutyCyclePercent(int channel, int duty) {
  setDutyCycle(channel, 0, int(duty * 4096 / 100));
}

void PCA9685Driver::setChannelLevel(int channel, int level) {
  if (level == 1) {
    setDutyCycle(channel, 0, 4095);
  } else {
    setDutyCycle(channel, 0, 0);
  }
}

