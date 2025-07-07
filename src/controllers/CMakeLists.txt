cmake_minimum_required(VERSION 3.10)

project(controllers LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -Wextra -ggdb")


set(CONTROLLERS_SRC_DIR ${CMAKE_SOURCE_DIR}/src/drivers)
set(CONTROLLERS_LIB_DIR ${CMAKE_SOURCE_DIR}/lib)
set(CONTROLLERS_INC_DIR ${CMAKE_SOURCE_DIR}/include)

set(TARGET controllers)

file(GLOB_RECURSE SRCS ${CONTROLLERS_SRC_DIR}/*.cpp ${CONTROLLERS_SRC_DIR}/*.h)
include_directories(${CONTROLLERS_INC_DIR})
find_library(WIRINGPI_LIB wiringPi PATHS ${CMAKE_SOURCE_DIR}/lib)

add_library(${TARGET} SHARED ${SRCS})
target_include_directories(${TARGET} PRIVATE ${CONTROLLERS_INC_DIR})
target_link_directories(${TARGET} PRIVATE  ${CONTROLLERS_LIB_DIR})
target_link_libraries(${TARGET} PRIVATE wiringPi
                                        mpg123
                                        vosk
                                        fmt
                                        spdlog
                                        espeak-ng
                                        piper
                                        piper_phonemize
                                        onnxruntime
                                        ALSA::ALSA
                                        Boost::system
                                        portaudio
                                        drivers
                                        ui
                                        )
