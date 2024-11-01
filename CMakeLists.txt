cmake_minimum_required(VERSION 3.13)
project(kufibot)

set(CMAKE_CXX_STANDARD 17) 
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(CMAKE_BUILD_TYPE Debug)

set(EXTERNAL_LIB_DIR "${CMAKE_SOURCE_DIR}/lib")


find_library(wiringPi_LIB wiringPi)

find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenCV REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ALSA REQUIRED)
find_package(CURL REQUIRED)

pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)

include_directories(${PORTAUDIO_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include) 
include_directories(${CURL_INCLUDE_DIRS})


add_executable(kufibot main.cpp 
                subscriber.h
                publisher.cpp
                publisher.h
                drivers/median_filter.h
                drivers/median_filter.cpp
                drivers/pca9685_driver.cpp 
                drivers/pca9685_driver.h 
                drivers/qmc5883l_driver.cpp
                drivers/qmc5883l_driver.h
                drivers/ina219_driver.cpp
                drivers/ina219_driver.h
                controllers/execution_controller.cpp
                controllers/execution_controller.h
                controllers/curl_controller.cpp
                controllers/curl_controller.h
                controllers/dc_motor_controller.cpp
                controllers/dc_motor_controller.h   
                controllers/servo_motor_controller.cpp
                controllers/servo_motor_controller.h
                controllers/distance_controller.cpp
                controllers/distance_controller.h
                controllers/compass_controller.cpp
                controllers/compass_controller.h
                controllers/power_controller.cpp
                controllers/power_controller.h
                controllers/speech_recognition_controller.cpp
                controllers/speech_recognition_controller.h
                controllers/speech_process_controller.cpp
                controllers/speech_process_controller.h
                services/service.cpp
                services/service.h
                services/remote_connection_service.cpp
                services/remote_connection_service.h
                services/video_stream_service.cpp
                services/video_stream_service.h
                services/web_socket_service.cpp
                services/web_socket_service.h
                services/robot_controller_service.cpp
                services/robot_controller_service.h
                services/interactive_chat_service.cpp
                services/interactive_chat_service.h
                services/mapping_service.cpp
                services/mapping_service.h
            )


target_link_directories(kufibot PUBLIC
${EXTERNAL_LIB_DIR}
)

target_include_directories(kufibot PUBLIC
${EXTERNAL_LIB_DIR}
)

target_link_libraries(kufibot ${wiringPi_LIB}
                              ${OpenCV_LIBS}
                              ${OPENSSL_LIBRARIES}
                              ${PORTAUDIO_LIBRARIES} 
                              ${CURL_LIBRARIES}
                              mpg123
                              vosk
                              fmt
                              spdlog
                              espeak-ng
                              piper
                              piper_phonemize
                              onnxruntime
                              ALSA::ALSA
                              Boost::system )