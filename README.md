# kufibot.cpp

## Overview
**kufibot.cpp** is a home made AI robot assistant C++ project designed for This guide provides instructions for setting up cross-compilation using Docker's buildx for multi-platform support.

---

## Mechinical Parts:
The mechincal parts are 3D printed. see:  https://www.printables.com/model/408363-wall-e

## Hardware Parts:
* 150 Rpm 12V DC Motor x 2
* mini servo motor x 6
* Usb Camera x1
* Usb wireless microphone x 1
* Bluetooth Speaker x 1
* HMI Display x 1
* 12V-5600 mah Li-On Battery Pack x 1
* INA219 i2c current cirtuit Sensor x 1
* QMC5883L i2c magnometer circuit x 1
* TF-Luna uart Lidar x 1
* Waveshare Hat DC Motor Driver x 1
* WaveShare Hat Servo Motor Driver x 1
* Raspberry Pi 5 x 1
* Hailo 8 AI Accelerator x 1

## SOFTWARE:
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
docker buildx build --platform linux/arm64 -t raspimage --load . 
```
* run compiled docker image
```bash
docker run --platform linux/arm64 --name test -it raspimage /bin/bash 
```

### Kufibot compilation
```bash
mkdir build && cd build
make
export LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH
```
  

