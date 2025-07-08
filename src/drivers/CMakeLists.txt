cmake_minimum_required(VERSION 3.10)

project(drivers LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -Wextra -ggdb")


set(DRIVERS_SRC_DIR ${CMAKE_SOURCE_DIR}/src/drivers)
set(DRIVERS_LIB_DIR ${CMAKE_SOURCE_DIR}/lib)
set(DRIVERS_INC_DIR ${CMAKE_SOURCE_DIR}/include)

set(TARGET drivers)
find_library(WIRINGPI_LIB wiringPi PATHS ${CMAKE_SOURCE_DIR}/lib)

file(GLOB_RECURSE SRCS ${DRIVERS_SRC_DIR}/*.cpp ${DRIVERS_SRC_DIR}/*.h)
include_directories(${DRIVERS_INC_DIR})

add_library(${TARGET} SHARED ${SRCS})
target_include_directories(${TARGET} PRIVATE ${DRIVERS_INC_DIR})
target_link_directories(${TARGET} PRIVATE  ${DRIVERS_LIB_DIR})
target_link_libraries(${TARGET} PRIVATE wiringPi)
install(TARGETS drivers DESTINATION /usr/local/lib)
