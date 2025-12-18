//
// Created by ybulb on 10/29/2025.
//

#ifndef DRIVER_DATA_H
#define DRIVER_DATA_H

struct Axis{
    float x,y,z, t;

    Axis (const Axis & other) {
        x = other.x;
        y = other.y;
        z = other.z;
    }

    Axis(){
        x = 0, y = 0, z = 0;
    }

    Axis& operator = (const Axis& other) {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }

    Axis& operator +=(const Axis& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Axis& operator / (int value) {
        x /= value;
        y /= value;
        z /= value;

        return *this;
    }
};


struct MPU6050Offset{
    Axis gyro;
    Axis accel;
};


struct MPU6050Data{
    Axis gyro;
    Axis accel;
};

struct BMP180Data{
    float pressure_pa;
    float temp_c;
};

struct INA219Data {
    float voltage;
    float shunt_voltage;
    float supply_voltage;
    float current;
    float power;
};


#endif //DRIVER_DATA_H
