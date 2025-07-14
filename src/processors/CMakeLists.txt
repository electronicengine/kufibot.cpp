cmake_minimum_required(VERSION 3.10)

project(processors LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -Wextra -ggdb")

set(PROCESSORS_SRC_DIR ${CMAKE_SOURCE_DIR}/src/processors)
set(PROCESSORS_LIB_DIR ${CMAKE_SOURCE_DIR}/lib)
set(PROCESSORS_INC_DIR ${CMAKE_SOURCE_DIR}/include)

set(TARGET processors)
set(PYTHON_INCLUDE_DIR /usr/include/python3.11)
set(PYTHON_LIBRARY /usr/lib/aarch64-linux-gnu/libpython3.11.so)

# OpenCV bul
find_package(OpenCV REQUIRED)

# Include directories
include_directories(${PYTHON_INCLUDE_DIR})
include_directories(${OpenCV_INCLUDE_DIRS})

file(GLOB_RECURSE SRCS ${PROCESSORS_SRC_DIR}/*.cpp ${PROCESSORS_SRC_DIR}/*.h)
include_directories(${PROCESSORS_INC_DIR})

add_library(${TARGET} SHARED ${SRCS})

# Linking
target_link_libraries(processors
        ${PYTHON_LIBRARY}
        ${OpenCV_LIBS}
)

# Compiler flags
target_compile_options(processors PRIVATE
        -Wall
        -Wextra
        -O2
)


install(TARGETS processors DESTINATION /usr/local/lib)