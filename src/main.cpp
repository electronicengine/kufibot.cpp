/*
 * This file is part of Kufibot.
 *
 * Kufibot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kufibot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kufibot. If not, see <https://www.gnu.org/licenses/>.
 */

#include "services/remote_connection_service.h"
#include "services/video_stream_service.h"
#include "services/robot_controller_service.h"
#include "services/web_socket_service.h"
#include "services/interactive_chat_service.h"
#include "services/tui_service.h"
#include "services/gesture_recognizer_service.h"
#include "services/gesture_performer_service.h"
#include "services/mapping_service.h"
#include "logger.h"
#include <string>


auto main(int argc, char *argv[]) -> int {
    bool useTui = false;
    bool startAllServices = false;
    int logLevel = 1;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--start-all-services") {
            startAllServices = true;
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

    Logger::init(nullptr, useTui, logLevel);

    if (startAllServices) {
        WebSocketService::get_instance()->start();
        VideoStreamService::get_instance()->start();
        RobotControllerService::get_instance()->start();
        RemoteConnectionService::get_instance()->start();
        InteractiveChatService::get_instance()->start();
        GesturePerformerService::get_instance()->start();
        GestureRecognizerService::get_instance()->start();
        MappingService::get_instance()->start();
    }

    TuiService *tui_service = TuiService::get_instance();
    tui_service->start();

    while (1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

}