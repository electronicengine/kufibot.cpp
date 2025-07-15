cmake_minimum_required(VERSION 3.10)

project(Tui LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -Wextra -ggdb")

set(TARGET tui)

set(TUI_SRC_DIR ${CMAKE_SOURCE_DIR}/src/tui)
set(TUI_LIB_DIR ${CMAKE_SOURCE_DIR}/lib)
set(TUI_INC_DIR ${CMAKE_SOURCE_DIR}/include)

file(GLOB_RECURSE SRCS ${TUI_SRC_DIR}/*.cpp ${TUI_SRC_DIR}/*.h)
include_directories(${TUI_INC_DIR})

add_library(${TARGET} SHARED ${SRCS})

target_include_directories(${TARGET} PRIVATE ${TUI_INC_DIR})
target_link_directories(${TARGET} PRIVATE  ${TUI_LIB_DIR})
target_link_libraries(${TARGET} PRIVATE final)
install(TARGETS ${TARGET} DESTINATION /usr/local/lib)
