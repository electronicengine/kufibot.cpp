
#include "services/remote_connection_service.h"
#include "services/video_stream_service.h"
#include "services/robot_controller_service.h"
#include "services/web_socket_service.h"
#include "services/interactive_chat_service.h"
#include "final/final.h"
#include "ui/main_window.h"
#include "ui/widget_color_theme.h"
#include "logger.h"
#include <iostream>
#include <string>


using namespace finalcut;

RemoteConnectionService *remote_connection_service;
VideoStreamService *video_stream_service;
RobotControllerService *robot_controller_service;
WebSocketService *web_socket_service;
InteractiveChatService *interactive_chat_service;

void commandLinePrompt() {
    while (1) {
        std::string input;
        std::cout << std::endl << "> ";
        std::getline(std::cin, input);
        if (input == "exit") {
            web_socket_service->stop();
            video_stream_service->stop();
            robot_controller_service->stop();
            remote_connection_service->stop();
            interactive_chat_service->stop();
            break;
        }else if(input == "sensors") {
            Logger::info(remote_connection_service->get_sensor_values());
        }else {
            interactive_chat_service->query(input, [](const std::string& message) {

                printf("%s", message.c_str());
                fflush(stdout);
            });
        }
    }
}

auto main(int argc, char *argv[]) -> int {
    bool useUI = true;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-no-tui") {
            useUI = false;
            break;
        }
    }

    Logger::init();
    remote_connection_service = RemoteConnectionService::get_instance();
    video_stream_service = VideoStreamService::get_instance();
    robot_controller_service = RobotControllerService::get_instance();
    web_socket_service = WebSocketService::get_instance();
    interactive_chat_service = InteractiveChatService::get_instance();

    web_socket_service->start("192.168.1.44", 8765);
    video_stream_service->start();
    robot_controller_service->start();
    remote_connection_service->start();
    interactive_chat_service->start();

    if (useUI) {
        FApplication app {argc, argv};
        app.setColorTheme<AWidgetColorTheme>();

        MainWindow main_dlg {&app};
        main_dlg.setText ("Log View");
        main_dlg.setGeometry (FPoint{1, 0}, FSize{FVTerm::getFOutput()->getColumnNumber(), FVTerm::getFOutput()->getLineNumber()});

        finalcut::FWidget::setMainWidget (&main_dlg);
        main_dlg.show();

        return app.exec();
    }else {
        commandLinePrompt();
        return 0;
    }
}