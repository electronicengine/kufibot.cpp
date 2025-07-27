
#include "services/remote_connection_service.h"
#include "services/video_stream_service.h"
#include "services/robot_controller_service.h"
#include "services/web_socket_service.h"
#include "services/interactive_chat_service.h"
#include "services/tui_service.h"
#include "services/gesture_recognizer_service.h"
#include "services/gesture_performing_service.h"
#include "final/final.h"
#include "logger.h"
#include <string>

#include "final/ftypes.h"
#include "final/ftypes.h"
#include "tui/widget_color_theme.h"



auto main(int argc, char *argv[]) -> int {
    bool useTui = true;

    GesturePerformingService::get_instance();

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--no-tui") {
            useTui = false;
            break;
        }
    }

    FApplication *app;
    MainWindow *main_window;
    if (useTui) {
        app = new FApplication{argc, argv};
        app->setColorTheme<AWidgetColorTheme>();
        main_window = new MainWindow{app};
        main_window->setText ("Log View");
        main_window->setGeometry (FPoint{1, 0}, FSize{FVTerm::getFOutput()->getColumnNumber(), FVTerm::getFOutput()->getLineNumber()});
        Logger::init(main_window, useTui);

        TuiService *tui_service = TuiService::get_instance(main_window, app, useTui);
        tui_service->start();
    }else {
        Logger::init(nullptr, useTui);

        TuiService *tui_service = TuiService::get_instance(nullptr, nullptr, useTui);
        tui_service->start();
    }

    GesturePerformingService::get_instance()->start();
    GestureRecognizerService::get_instance()->start();
    RemoteConnectionService::get_instance()->disable();
    RobotControllerService::get_instance()->disable();
    WebSocketService::get_instance()->disable();
    InteractiveChatService::get_instance()->start();
    VideoStreamService::get_instance()->start();

    while (1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

}