
#include "services/remote_connection_service.h"
#include "services/video_stream_service.h"
#include "services/robot_controller_service.h"
#include "services/web_socket_service.h"
#include "services/interactive_chat_service.h"
#include "services/tui_service.h"
#include "final/final.h"
#include "logger.h"
#include <iostream>
#include <string>

#include "services/tui_service.h"



auto main(int argc, char *argv[]) -> int {
    Logger::init();

    RemoteConnectionService::get_instance()->start();
    VideoStreamService::get_instance()->start();
    RobotControllerService::get_instance()->start();
    WebSocketService::get_instance()->start();
    InteractiveChatService::get_instance()->start();

    TuiService *tui_service = TuiService::get_instance(argc, argv);
    tui_service->start();

    while (1) {
        sleep(1);
    }
}