cmake_minimum_required(VERSION 3.13)
project(kufibot)

add_compile_options(-Wno-unused-function)
add_compile_options(-Wno-unused-parameter)

set(CMAKE_CXX_STANDARD 17)
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(CMAKE_BUILD_TYPE Debug)

set(EXTERNAL_LIB_DIR "${CMAKE_SOURCE_DIR}/lib")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

# Deployment configuration
set(DEPLOY_USER phi)
set(DEPLOY_HOST 192.168.191.1)
set(DEPLOY_PATH /home/phi/workspace/)

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

add_executable(kufibot  ${SRC_DIR}/main.cpp ${SRCS}
        src/controllers/llama_controller.cpp
        src/controllers/llama_controller.h)


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
        fmt
        spdlog
        espeak-ng
        onnxruntime
        ALSA::ALSA
        Boost::system
        portaudio
        piper
        piper_phonemize
        vosk
        mpg123
        onnxruntime
        final
        ui
        drivers
        controllers
        services
        llama
        common
        ggml
        ggml-base
        ggml-cpu
)

include(InstallRequiredSystemLibraries)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "kufibot.cpp")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_CONTACT "mail@yusufbulbul.com")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Yusuf Bülbül")  # required
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.27)")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

include(CPack)

install(TARGETS kufibot DESTINATION /usr/local/bin)
install(DIRECTORY include/ DESTINATION /usr/local/include)
install(DIRECTORY lib/ DESTINATION /usr/local/lib)

#add_custom_command(
#        TARGET kufibot
#        POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E echo "Generating .deb package with CPack..."
#        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR} cpack -G DEB
#        COMMAND ${CMAKE_COMMAND} -E echo "Deploying .deb package to ${DEPLOY_USER}@${DEPLOY_HOST}:${DEPLOY_PATH}"
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/kufibot.cpp-1.0.0-Linux.deb ${CMAKE_BINARY_DIR}/kufibot.deb
#        COMMAND scp ${CMAKE_BINARY_DIR}/kufibot.deb ${DEPLOY_USER}@${DEPLOY_HOST}:${DEPLOY_PATH}/
#        COMMAND ssh ${DEPLOY_USER}@${DEPLOY_HOST} "sudo dpkg -i ${DEPLOY_PATH}/kufibot.deb"
#        COMMENT "Packaging and deploying kufibot.deb to target"
#)