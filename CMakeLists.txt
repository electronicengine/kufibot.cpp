cmake_minimum_required(VERSION 3.13)
project(kufibot)

set(CMAKE_CXX_STANDARD 17) 
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(CMAKE_BUILD_TYPE Debug)

set(EXTERNAL_LIB_DIR "${CMAKE_SOURCE_DIR}/lib")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenCV REQUIRED)
find_package(ALSA REQUIRED)
find_package(CURL REQUIRED)

include_directories(${PORTAUDIO_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include) 
include_directories(${CURL_INCLUDE_DIRS})

add_subdirectory(src/ui)
add_subdirectory(src/drivers)
add_subdirectory(src/controllers)
add_subdirectory(src/services)


file(GLOB_RECURSE SRCS 
                    ${SRC_DIR}/subscriber.h
                    ${SRC_DIR}/publisher.cpp
                    ${SRC_DIR}/publisher.h
                    ${SRC_DIR}/controllers/*.cpp
                    ${SRC_DIR}/controllers/*.h
                    ${SRC_DIR}/services/*.cpp
                    ${SRC_DIR}/services/*.h
                )




add_executable(kufibot  ${SRC_DIR}/main.cpp ${SRCS})


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
                              wiringPi
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
                              final
                              ui
                              drivers
                              controllers
                              services
                              )
