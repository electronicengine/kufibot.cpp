cmake_minimum_required(VERSION 3.10)

project(operators LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -Wextra -ggdb")
set(TARGET operators)

set(OPERATORS_SRC_DIR ${CMAKE_SOURCE_DIR}/src/operators)
set(OPERATORS_LIB_DIR ${CMAKE_SOURCE_DIR}/lib)
set(OPERATORS_INC_DIR ${CMAKE_SOURCE_DIR}/include)

set(PYTHON_INCLUDE_DIR /usr/include/python3.11)
set(PYTHON_LIBRARY /usr/lib/aarch64-linux-gnu/libpython3.11.so)

# OpenCV bul
find_package(OpenCV REQUIRED)

# Include directories
include_directories(${PYTHON_INCLUDE_DIR})
include_directories(${OpenCV_INCLUDE_DIRS})

file(GLOB_RECURSE SRCS ${OPERATORS_SRC_DIR}/*.cpp ${OPERATORS_SRC_DIR}/*.h)
include_directories(${OPERATORS_INC_DIR})

add_library(${TARGET} SHARED ${SRCS})

# Linking
target_link_libraries(${TARGET}
        ${PYTHON_LIBRARY}
        ${OpenCV_LIBS}
)

# Compiler flags
target_compile_options(${TARGET} PRIVATE
        -Wall
        -Wextra
        -O2
)


install(TARGETS ${TARGET} DESTINATION /usr/local/lib)
install(FILES face_gesture_recognition_module.py DESTINATION /usr/local/bin)
install(FILES hand_gesture_recognition_module.py DESTINATION /usr/local/bin)
