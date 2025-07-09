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
#include "controllers/curl_controller.h"
#include "final/final.h"
#include "ui/main_window.h"
#include "ui/widget_color_theme.h"
#include <iostream>
#include <curl/curl.h>
#include <string>
#include <sstream>

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using namespace finalcut;


auto main (int argc, char* argv[]) -> int
{

    RemoteConnectionService *remote_connection_service = RemoteConnectionService::get_instance();
    VideoStreamService *video_stream_service = VideoStreamService::get_instance();
    RobotControllerService *robot_controller_service = RobotControllerService::get_instance();
    WebSocketService *web_socket_service = WebSocketService::get_instance();
    InteractiveChatService *interactive_chat_service = InteractiveChatService::get_instance();

    web_socket_service->start("192.168.1.44", 8765);
    video_stream_service->start();
    robot_controller_service->start();
    remote_connection_service->start();
    interactive_chat_service->start();

    FApplication app {argc, argv};
    app.setColorTheme<AWidgetColorTheme>();

    MainWindow main_dlg {&app};
    main_dlg.setText ("Log View");
    main_dlg.setGeometry (FPoint{1, 0}, FSize{FVTerm::getFOutput()->getColumnNumber(), FVTerm::getFOutput()->getLineNumber()});

    finalcut::FWidget::setMainWidget (&main_dlg);
    main_dlg.show();

    return app.exec();

}