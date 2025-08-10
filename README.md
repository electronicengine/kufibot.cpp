# kufibot.cpp

## Overview
**kufibot.cpp** is a home made AI  assistant robot writen in C++.  This guide provides instructions for setting up cross-compilation using Docker's buildx for multi-platform support.

---

## Mechinical
The mechanical parts are 3D printed. see:  https://www.printables.com/model/408363-wall-e

## Hardware
* 150 Rpm 12V DC Motor x 2
* mini servo motor x 6
* Usb Camera x 1
* Usb wireless microphone x 1
* Bluetooth Speaker x 1
* HMI Display x 1
* 3S - 5600 mah Li-On Battery Pack x 1
* INA219 i2c current sensor cirtuit Sensor x 1
* 10dof i2c magnometer and accelorometer circuit x 1
* TF-Luna uart Lidar circuit x 1
* Waveshare Hat DC Motor Driver circuit x 1
* WaveShare Hat Servo Motor Driver circuit x 1
* Raspberry Pi 5 x 1
* Hailo 8 AI Accelerator x 1

## Software
### Enable Multi-Architecture Support
To enable multi-architecture support, run the following commands:

* Initialize QEMU for multi-architecture emulation
```bash
docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
```
* Create a Docker builder instance for multi-platform support
```bash
docker buildx create --use --name mybuilder
```
* Bootstrap the builder
```bash
docker buildx inspect mybuilder --bootstrap
```
### Compile and Run Arm64 Docker
* Compile docker with Dockerfile and load image
```bash
docker compose -f docker-compose.yml build 
```
* run compiled docker image
```bash
docker compose -f docker-compose.yml up -d 
docker exec -it kufibot_env bash 
```
### Compile and Run Amd64 Cross Compilation Docker
* Compile docker with Dockerfile and load image
```bash
docker compose -f docker-compose.cross.yml build 
```
* run compiled docker image
```bash
docker compose -f docker-compose.cross.yml up -d 
docker exec -it kufibot_cross_env bash 
```

### Kufibot Compilation
```bash
mkdir build && cd build
cmake ..
make
export LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH
```

## License

This project is licensed under the GNU General Public License version 3 (GPLv3).

You can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but **WITHOUT ANY WARRANTY**; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

For the full license text, see the [LICENSE](./LICENSE) file.

---

### More about GPLv3

The GPLv3 license ensures that all copies and derivatives of this project remain free and open source under the same license.

