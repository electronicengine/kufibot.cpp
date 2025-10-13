cmake_minimum_required(VERSION 3.10)

project(statemachines LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -Wextra -ggdb")

set(STATEMACHINES_SRC_DIR ${CMAKE_SOURCE_DIR}/src/statemachines)
set(STATEMACHINES_LIB_DIR ${CMAKE_SOURCE_DIR}/lib)
set(STATEMACHINES_INC_DIR ${CMAKE_SOURCE_DIR}/include)

set(TARGET statemachines)

file(GLOB_RECURSE SRCS ${STATEMACHINES_SRC_DIR}/*.cpp ${STATEMACHINES_SRC_DIR}/*.h)
include_directories(${STATEMACHINES_INC_DIR})
include_directories(${CMAKE_SOURCE_DIR}/operators/include)

add_library(${TARGET} SHARED ${SRCS})

target_include_directories(${TARGET} PRIVATE ${STATEMACHINES_INC_DIR})
target_link_directories(${TARGET} PRIVATE  ${STATEMACHINES_LIB_DIR})
target_link_libraries(${TARGET} PRIVATE controllers drivers operators)
install(TARGETS statemachines DESTINATION /usr/local/lib)
