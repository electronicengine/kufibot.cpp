
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
    int logLevel = 1;

    GesturePerformingService* gesturePerformingService = GesturePerformingService::get_instance();
    GestureRecognizerService* gestureRecognizerService = GestureRecognizerService::get_instance();
    RemoteConnectionService* remoteConnectionService = RemoteConnectionService::get_instance();
    RobotControllerService* robotControllerService = RobotControllerService::get_instance();
    WebSocketService* webSocketService = WebSocketService::get_instance();
    InteractiveChatService* interactiveChatService = InteractiveChatService::get_instance();
    VideoStreamService* videoStreamService = VideoStreamService::get_instance();

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--no-tui") {
            useTui = false;
            break;
        }
        if (arg == "--log-level") {
            if (argv[i+1] == "trace") {
                logLevel = 0;
            }else if (argv[i+1] == "debug") {
                logLevel = 1;
            }else if (argv[i+1] == "info") {
                logLevel = 2;
            }else if (argv[i+1] == "warn") {
                logLevel = 3;
            }else if (argv[i+1] == "error") {
                logLevel = 4;
            }
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
        Logger::init(main_window, useTui, logLevel);

        TuiService *tui_service = TuiService::get_instance(main_window, app, useTui);
        tui_service->start();
    }else {
        Logger::init(nullptr, useTui, logLevel);

        TuiService *tui_service = TuiService::get_instance(nullptr, nullptr, useTui);
        tui_service->start();
    }

    gesturePerformingService->start();
    gestureRecognizerService->disable();
    remoteConnectionService->disable();
    robotControllerService->disable();
    webSocketService->disable();
    interactiveChatService->disable();
    videoStreamService->disable();

    while (1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

}