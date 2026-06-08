#ifndef HMC5883L_DRIVER_H
#define HMC5883L_DRIVER_H

#include "driver_data.h"
#include "i2c_device.h"


// HMC5883L register addresses
const int HMC5883L_ADDRESS = 0x1E;
const int HMC5883L_CONFIG_A = 0x00;
const int HMC5883L_CONFIG_B = 0x01;
const int HMC5883L_MODE = 0x02;
const int HMC5883L_X_MSB = 0x03;
const int HMC5883L_Z_MSB = 0x05;
const int HMC5883L_Y_MSB = 0x07;

# define M_PI		3.14159265358979323846	/* pi */

class HMC5883LDriver : public I2CDevice {
private:
    int fd;

    static double compute_heading(double x, double y);
    int16_t read_x();
    int16_t read_y();
    int16_t read_z();

public:
    HMC5883LDriver();
    ~HMC5883LDriver() override;

    bool initialize() override;
    void shutdown() override;

    bool initHMC5883l();
    Axis readHMC5883l();


};

#endif // HMC5883L_DRIVER_H
