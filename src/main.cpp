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

#include "services/landmark_tracker_service.h"


auto main(int argc, char *argv[]) -> int {
    bool showFrame = false;
    bool useTui = false;
    bool asService = false;
    bool stopAllServices = false;
    int logLevel = 1;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help") {
            std::cout << "Usage: kufibot [--as-service] [--use-tui] [--stop-all-services] [--show-frame] [--log-level <level>]" << std::endl;
            return 0;
        }
        if (arg == "--stop-all-services") {
            stopAllServices = true;
            break;
        }
        if (arg == "--show-frame") {
            showFrame = true;
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
        if (arg == "--as-service") {
            asService = true;
        }
    }

    Logger::init(nullptr, useTui, logLevel);

    if (!stopAllServices) {
        bool ret;
        ret = GesturePerformerService::get_instance()->start();
        if (!ret) {
            return 1;
        }
        ret = GestureRecognizerService::get_instance(showFrame)->start();
        if (!ret) {
            return 1;
        }
        ret = WebSocketService::get_instance()->start();
        if (!ret) {
            return 1;
        }
        ret = RobotControllerService::get_instance()->start();
        if (!ret) {
            return 1;
        }
        ret = RemoteConnectionService::get_instance()->start();
        if (!ret) {
            return 1;
        }
        ret = VideoStreamService::get_instance()->start();
        if (!ret) {
            return 1;
        }
        ret = InteractiveChatService::get_instance()->start();
        if (!ret) {
            return 1;
        }
        ret = LandmarkTrackerService::get_instance()->start();
        if (!ret) {
            return 1;
        }
        
        // ret = MappingService::get_instance()->stop();
        // if (!ret) {
        //     return 1;
        // }
    }

    if (!asService) {
        bool ret = TuiService::get_instance()->start();
        if (!ret) {
            return 1;
        }
    }

    while (1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

}
