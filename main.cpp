#include <iostream>
#include <fstream>
#include <string>
#include <functional>
#include <csignal>

#include "services/remote_connection_service.h"
#include "services/video_stream_service.h"
#include "services/robot_controller_service.h"
#include "services/web_socket_service.h"
#include "services/interactive_chat_service.h"
#include "services/mapping_service.h"

#include "controllers/compass_controller.h"
#include "controllers/distance_controller.h"


int main() {

    RemoteConnectionService *remote_connection_service = RemoteConnectionService::get_instance();
    VideoStreamService *video_stream_service = VideoStreamService::get_instance();
    RobotControllerService *robot_controller_service = RobotControllerService::get_instance();
    WebSocketService *web_socket_service = WebSocketService::get_instance();
    InteractiveChatService *interactive_chat_service = InteractiveChatService::get_instance();
    // MappingService *mapping_service = MappingService::get_instance();

    web_socket_service->start("192.168.1.44", 8765);
    video_stream_service->start();
    robot_controller_service->start();
    remote_connection_service->start();
    interactive_chat_service->start();
    // mapping_service->start();

    
    std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    std::condition_variable cv;
    cv.wait(lock);

    return 0;

}