#include <iostream>
#include <fstream>
#include <string>
#include <functional>
#include <csignal>

#include "services/remote_connection_service.h"
#include "services/video_stream_service.h"
#include "services/robot_controller_service.h"
#include "services/web_socket_service.h"


int main() {


    // Create a FrameProcessor observer and register it
    RemoteConnectionService *remote_connection_service = RemoteConnectionService::get_instance();
    VideoStreamService *video_stream_service = VideoStreamService::get_instance();
    RobotControllerService *robot_controller_service = RobotControllerService::get_instance();
    WebSocketService *web_socket_service = WebSocketService::get_instance();
    
    web_socket_service->start("192.168.1.44", 8765);
    video_stream_service->start();
    robot_controller_service->start();
    remote_connection_service->start();

    std::cout << "Press any key to stop the program." << std::endl;
    std::cin.get();  // Wait for user input

    return 0;

}