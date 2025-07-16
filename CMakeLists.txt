cmake_minimum_required(VERSION 3.13)
project(kufibot)

add_compile_options(-Wno-unused-function)
add_compile_options(-Wno-unused-parameter)

set(CMAKE_CXX_STANDARD 17)
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(CMAKE_BUILD_TYPE Debug)

set(EXTERNAL_LIB_DIR "${CMAKE_SOURCE_DIR}/lib")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

# Deployment configuration
set(DEPLOY_USER kufi)
set(DEPLOY_HOST 192.168.1.44)
set(DEPLOY_PATH /home/kufi/workspace/)

set(PYTHON_INCLUDE_DIR /usr/include/python3.11)
set(PYTHON_LIBRARY /usr/lib/aarch64-linux-gnu/libpython3.11.so)


find_package(Boost REQUIRED CONFIG COMPONENTS system filesystem)
find_package(OpenCV REQUIRED)
find_package(ALSA REQUIRED)
find_package(CURL REQUIRED)

include_directories(${PORTAUDIO_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CURL_INCLUDE_DIRS})
include_directories(${PYTHON_INCLUDE_DIR})

add_subdirectory(src/tui)
add_subdirectory(src/drivers)
add_subdirectory(src/controllers)
add_subdirectory(src/services)
add_subdirectory(src/operators)

file(GLOB_RECURSE SRCS
        ${SRC_DIR}/subscriber.h
        ${SRC_DIR}/publisher.cpp
        ${SRC_DIR}/publisher.h
        ${SRC_DIR}/controllers/*.cpp
        ${SRC_DIR}/controllers/*.h
        ${SRC_DIR}/services/*.cpp
        ${SRC_DIR}/services/*.h
        ${SRC_DIR}/operators/*.cpp
        ${SRC_DIR}/operators/*.h
)

add_executable(kufibot  ${SRC_DIR}/main.cpp ${SRCS}
        src/public_data_messages.h)


target_link_directories(kufibot PUBLIC
        ${EXTERNAL_LIB_DIR}
)

target_include_directories(kufibot PUBLIC
        ${EXTERNAL_LIB_DIR}
)

target_link_libraries(kufibot ${OpenCV_LIBS}
        ${OPENSSL_LIBRARIES}
        ${PORTAUDIO_LIBRARIES}
        ${CURL_LIBRARIES}
        ${PYTHON_LIBRARY}
        fmt
        spdlog
        espeak-ng
        onnxruntime
        ALSA::ALSA
        Boost::system
        portaudio
        piper
        piper_phonemize
        vosk
        mpg123
        onnxruntime
        final
        tui
        drivers
        controllers
        services
        operators
        llama
        common
        ggml
        ggml-base
        ggml-cpu
)

include(InstallRequiredSystemLibraries)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "kufibot.cpp")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_CONTACT "mail@yusufbulbul.com")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Yusuf Bülbül")  # required
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.27)")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

include(CPack)

install(TARGETS kufibot DESTINATION /usr/local/bin)

#add_custom_command(
#        TARGET kufibot
#        POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E echo "Generating .deb package with CPack..."
#        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR} cpack -G DEB
#        COMMAND ${CMAKE_COMMAND} -E echo "Deploying .deb package to ${DEPLOY_USER}@${DEPLOY_HOST}:${DEPLOY_PATH}"
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/kufibot.cpp-1.0.0-Linux.deb ${CMAKE_BINARY_DIR}/kufibot.deb
#        COMMAND scp ${CMAKE_BINARY_DIR}/kufibot.deb ${DEPLOY_USER}@${DEPLOY_HOST}:${DEPLOY_PATH}/
#        COMMAND ssh ${DEPLOY_USER}@${DEPLOY_HOST} "sudo dpkg -i ${DEPLOY_PATH}/kufibot.deb"
#        COMMENT "Packaging and deploying kufibot.deb to target"
#)