# arm64Build.cmake - Cross-compilation toolchain for ARM64 (Raspberry Pi)

# Basic system configuration
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Cross-compiler settings
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# Target environment paths
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu /usr/lib/aarch64-linux-gnu /usr/include/aarch64-linux-gnu)
# Remove CMAKE_SYSROOT as it's causing linker issues
# set(CMAKE_SYSROOT /usr/aarch64-linux-gnu)

# Fix the library path issue
set(CMAKE_LIBRARY_PATH /usr/lib/aarch64-linux-gnu)
set(CMAKE_INCLUDE_PATH /usr/include/aarch64-linux-gnu)

# Search behavior
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Additional compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function -Wno-unused-parameter")

# OpenSSL configuration
set(OPENSSL_ROOT_DIR /usr/aarch64-linux-gnu)
set(OPENSSL_INCLUDE_DIR /usr/include/aarch64-linux-gnu)
set(OPENSSL_CRYPTO_LIBRARY /usr/lib/aarch64-linux-gnu/libcrypto.so)
set(OPENSSL_SSL_LIBRARY /usr/lib/aarch64-linux-gnu/libssl.so)

# Boost configuration
set(Boost_NO_SYSTEM_PATHS ON)
set(BOOST_ROOT /usr/aarch64-linux-gnu)
set(BOOST_INCLUDEDIR /usr/include/aarch64-linux-gnu)
set(BOOST_LIBRARYDIR /usr/lib/aarch64-linux-gnu)

# OpenCV configuration
set(OpenCV_DIR /usr/lib/aarch64-linux-gnu/cmake/opencv4)

# ALSA configuration
set(ALSA_INCLUDE_DIR /usr/include/aarch64-linux-gnu)
set(ALSA_LIBRARY /usr/lib/aarch64-linux-gnu/libasound.so)

# cURL configuration
set(CURL_INCLUDE_DIR /usr/include/aarch64-linux-gnu)
set(CURL_LIBRARY /usr/lib/aarch64-linux-gnu/libcurl.so)

# PortAudio configuration
set(PORTAUDIO_INCLUDE_DIR /usr/include/aarch64-linux-gnu)
set(PORTAUDIO_LIBRARY /usr/lib/aarch64-linux-gnu/libportaudio.so)

# D-Bus configuration
set(DBUS_INCLUDE_DIR /usr/include/aarch64-linux-gnu/dbus-1.0)
set(DBUS_LIBRARY /usr/lib/aarch64-linux-gnu/libdbus-1.so)

# GLib configuration
set(GLIB_INCLUDE_DIR /usr/include/aarch64-linux-gnu/glib-2.0)
set(GLIB_LIBRARY /usr/lib/aarch64-linux-gnu/libglib-2.0.so)

# MPG123 configuration
set(MPG123_INCLUDE_DIR /usr/include/aarch64-linux-gnu)
set(MPG123_LIBRARY /usr/lib/aarch64-linux-gnu/libmpg123.so)

# Help CMake find the cross-compiled packages
set(PKG_CONFIG_EXECUTABLE /usr/bin/aarch64-linux-gnu-pkg-config)

# Set PKG_CONFIG_PATH for cross-compilation
set(ENV{PKG_CONFIG_PATH} /usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig)

# Fix linker issues by setting proper library paths
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath-link,/usr/lib/aarch64-linux-gnu")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath-link,/usr/lib/aarch64-linux-gnu")

# Additional CMake module paths
list(APPEND CMAKE_MODULE_PATH /usr/lib/aarch64-linux-gnu/cmake)
list(APPEND CMAKE_PREFIX_PATH /usr/lib/aarch64-linux-gnu/cmake)

# Compiler and linker flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a")

# Ensure we use the correct runtime path
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "/usr/lib/aarch64-linux-gnu")
