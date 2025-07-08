cmake_minimum_required(VERSION 3.10)

project(ui LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -Wextra -ggdb")


set(UI_SRC_DIR ${CMAKE_SOURCE_DIR}/src/ui)
set(UI_LIB_DIR ${CMAKE_SOURCE_DIR}/lib)
set(UI_INC_DIR ${CMAKE_SOURCE_DIR}/include)

set(TARGET ui)

file(GLOB_RECURSE SRCS ${UI_SRC_DIR}/*.cpp ${UI_SRC_DIR}/*.h)
include_directories(${UI_INC_DIR})

add_library(${TARGET} SHARED ${SRCS})
target_include_directories(${TARGET} PRIVATE ${UI_INC_DIR})
target_link_directories(${TARGET} PRIVATE  ${UI_LIB_DIR})
target_link_libraries(${TARGET} PRIVATE final)
install(TARGETS ui DESTINATION /usr/local/lib)
