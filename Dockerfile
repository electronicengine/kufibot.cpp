# Use the official Raspberry Pi OS (Debian-based) as the base image
FROM arm64v8/debian:bookworm

# Set environment variable to avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    nano \
    cmake \
    git \
    gcc \
    pkg-config \
    libboost-all-dev \
    libssl-dev \
    libdbus-1-dev \
    libasound2-dev \
    libaudio-dev \
    libopencv-dev \
    portaudio19-dev \
    libcurl4-openssl-dev \
    mpg123 \
    libmpg123-dev && \
    # Clean up APT when done to reduce image size
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# You can copy your application files here if needed
RUN git clone https://github.com/electronicengine/kufibot.cpp.git /app/kufibot.cpp

# Set the working directory
WORKDIR /app/kufibot.cpp


