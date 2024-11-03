# kufibot.cpp

# For Cross Compilation
* cd kufibot.cpp
* docker buildx build --platform linux/arm64 -t raspimage --load .  //compile docker
* docker run --platform linux/arm64 --name test -it raspimage /bin/bash // run the docker

# Compilation
* cd kufibot.cpp
* mkdir build && cd build
* make

  

