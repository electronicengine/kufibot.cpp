# kufibot.cpp

## Overview
**kufibot.cpp** is a home made robot assistant C++ project designed for This guide provides instructions for setting up cross-compilation using Docker's buildx for multi-platform support.

---

## Cross-Compilation Setup

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
```bash
docker buildx build --platform linux/arm64 -t raspimage --load .  //compile docker
docker run --platform linux/arm64 --name test -it raspimage /bin/bash // run the docker
```

## Kufibot compilation
```bash
cd kufibot.cpp
mkdir build && cd build
make
```
  

