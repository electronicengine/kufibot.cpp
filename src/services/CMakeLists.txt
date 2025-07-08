cmake_minimum_required(VERSION 3.10)

project(services LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -Wextra -ggdb")


set(SERVICES_SRC_DIR ${CMAKE_SOURCE_DIR}/src/services)
set(SERVICES_LIB_DIR ${CMAKE_SOURCE_DIR}/lib)
set(SERVICES_INC_DIR ${CMAKE_SOURCE_DIR}/include)

set(TARGET services)

file(GLOB_RECURSE SRCS ${SERVICES_SRC_DIR}/*.cpp ${SERVICES_SRC_DIR}/*.h)
include_directories(${SERVICES_INC_DIR})

add_library(${TARGET} SHARED ${SRCS})
target_include_directories(${TARGET} PRIVATE ${SERVICES_INC_DIR})
target_link_directories(${TARGET} PRIVATE  ${SERVICES_LIB_DIR})
target_link_libraries(${TARGET} PRIVATE ui
                                        controllers
                                        )
install(TARGETS services DESTINATION /usr/local/lib)
