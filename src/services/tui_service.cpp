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

#include "tui_service.h"
#include "web_socket_service.h"
#include "robot_controller_service.h"
#include "remote_connection_service.h"
#include "video_stream_service.h"
#include "gesture_performer_service.h"
#include "mapping_service.h"
#include "interactive_chat_service.h"
#include "robot_controller_service.h"

#undef K
#undef null

#include <oneapi/tbb/detail/_range_common.h>

#include "gesture_recognizer_service.h"
#include "../logger.h"
#include "final/final.h"
#include "../tui/main_window.h"
#include "../tui/widget_color_theme.h"

using namespace finalcut;


TuiService* TuiService::_instance = nullptr;


TuiService *TuiService::get_instance()
{

    if (_instance == nullptr) {
        _instance = new TuiService();
    }
    return _instance;
}

TuiService::TuiService() : Service("TuiService") {}

bool TuiService::initialize() {
    _tuiLlmResponseCallBackFunction = [this](const std::string& response) {
        INFO("{}", response);
    };

    subscribe_to_service(RobotControllerService::get_instance());
    subscribe_to_service(InteractiveChatService::get_instance());
    subscribe_to_service(GestureRecognizerService::get_instance());

    return true;
}


void TuiService::service_function() {

    while (_running) {
        std::string input;
        std::cout << std::endl << "> ";
        std::getline(std::cin, input);
        if (input.empty()) {
            continue;
        }else if (input == "exit") {
            WebSocketService::get_instance()->stop();
            VideoStreamService::get_instance()->stop();
            RobotControllerService::get_instance()->stop();
            RemoteConnectionService::get_instance()->stop();
            InteractiveChatService::get_instance()->stop();
            GesturePerformerService::get_instance()->stop();
            GestureRecognizerService::get_instance()->stop();
            MappingService::get_instance()->stop();

            std::exit(0);
        }else if (input =="openTui") {
            openTui();
        }else if (input == "help") {
            printHelp();
        }else if (input == "startAll") {
            WebSocketService::get_instance()->start();
            VideoStreamService::get_instance()->start();
            RobotControllerService::get_instance()->start();
            RemoteConnectionService::get_instance()->start();
            InteractiveChatService::get_instance()->start();
            GesturePerformerService::get_instance()->start();
            GestureRecognizerService::get_instance()->start();
            MappingService::get_instance()->start();
        }else if (input.find("start") != std::string::npos) {
            if (input.find("RobotControllerService") != std::string::npos) {
                RobotControllerService::get_instance()->start();
            }else if (input.find("InteractiveChatService") != std::string::npos) {
                InteractiveChatService::get_instance()->start();
            }else if (input.find("RemoteConnectionService") != std::string::npos) {
                RemoteConnectionService::get_instance()->start();
            }else if (input.find("VideoStreamService") != std::string::npos) {
                VideoStreamService::get_instance()->start();
            }else if (input.find("MappingService") != std::string::npos) {
                MappingService::get_instance()->start();
            }else if (input.find("GesturePerformerService") != std::string::npos) {
                GesturePerformerService::get_instance()->start();
            }else if (input.find("GestureRecognizerService") != std::string::npos) {
                GestureRecognizerService::get_instance()->start();
            }else if (input.find("WebSocketService") != std::string::npos) {
                WebSocketService::get_instance()->start();
            }
        }else if (input.find("stop") != std::string::npos) {
            if (input.find("RobotControllerService") != std::string::npos) {
                RobotControllerService::get_instance()->stop();
            }else if (input.find("InteractiveChatService") != std::string::npos) {
                InteractiveChatService::get_instance()->stop();
            }else if (input.find("RemoteConnectionService") != std::string::npos) {
                RemoteConnectionService::get_instance()->stop();
            }else if (input.find("VideoStreamService") != std::string::npos) {
                VideoStreamService::get_instance()->stop();
            }else if (input.find("MappingService") != std::string::npos) {
                MappingService::get_instance()->stop();
            }else if (input.find("GesturePerformerService") != std::string::npos) {
                GesturePerformerService::get_instance()->stop();
            }else if (input.find("GestureRecognizerService") != std::string::npos) {
                GestureRecognizerService::get_instance()->stop();
            }else if (input.find("WebSocketService") != std::string::npos) {
                WebSocketService::get_instance()->stop();
            }
        }else if (input.find("log" ) != std::string::npos) {
            std::string className = input.substr(4, input.size() - 4);
            Logger::print_cached_logs(className);
        }else if(input == "sensors") {
            INFO("{} ",_currentSensorData.to_json());
        }else if (input.find("gestures") != std::string::npos) {
            int seconds = std::stoi(input.substr(9, input.size() - 9));
            printGestureData(seconds);
        }else if (input.find("llm") != std::string::npos) {
            tui_llm_query_callback(std::string(input.begin() + 4, input.end()));
        }else if (input.find("set") != std::string::npos) {
            if (RobotControllerService::get_instance()->is_running()) {
                std::istringstream iss(input);
                std::string command;
                std::string joint;
                int angle;
                iss >> command >> joint >> angle;
                if (joint == "rightArm") {
                    setJointAngle(ServoMotorJoint::rightArm, angle);
                }else if (joint == "leftArm") {
                    setJointAngle(ServoMotorJoint::leftArm, angle);
                }else if (joint == "neck") {
                    setJointAngle(ServoMotorJoint::neck, angle);
                }else if (joint == "headUpDown") {
                    setJointAngle(ServoMotorJoint::headUpDown, angle);
                }else if (joint == "headLeftRight") {
                    setJointAngle(ServoMotorJoint::headLeftRight, angle);
                }else if (joint == "eyeLeft") {
                    setJointAngle(ServoMotorJoint::eyeLeft, angle);
                }else if (joint == "eyeRight") {
                    setJointAngle(ServoMotorJoint::eyeRight, angle);
                }else {
                    WARNING("There is no joint with this name");
                }

            }else {
                WARNING("Start robot controller service first!");
            }
        }
    }
}

void TuiService::printGestureData(int seconds) {
    INFO("Gesture Data: ");
    seconds = seconds * 1000;
    while (seconds > 0) {
        std::cout << "Recognized Face Gesture: " << _recognizedGestureData.faceEmotion << std::endl;
        std::cout << "Recognized Hand Gesture: " << _recognizedGestureData.handGesture << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        seconds -= 100;
    }

}

void TuiService::openTui() {

    int argc = 0;
    char **argv = nullptr;

    FApplication *app;
    MainWindow *main_window;

    app = new FApplication{argc, argv};
    app->setColorTheme<AWidgetColorTheme>();
    main_window = new MainWindow{app};
    main_window->setText ("Log View");
    main_window->setGeometry (FPoint{1, 0}, FSize{FVTerm::getFOutput()->getColumnNumber(), FVTerm::getFOutput()->getLineNumber()});

    _tuiSensorCallBackFunction = main_window->_measurementsWindow-> get_sensor_data_callback_function();
    _tuiLlmResponseCallBackFunction = main_window->_chatWindow->get_llm_response_callback_function();
    _tuiCompasDirectionCallBackFunction = main_window->_compassRTGraphWindow->get_compas_direction_callback_function();
    _tuiMotorFeedBackInfoCallBackFunction = main_window->_bodyControllerWindow->get_servo_joints_callback_function();

    //set tui control CallBack Functions
    main_window->_chatWindow->set_llm_query_function_callback(std::bind(&TuiService::tui_llm_query_callback, this, std::placeholders::_1));
    main_window->_bodyControllerWindow->setControlFunctionCallBack(std::bind(&TuiService::tui_control_function_callback, this, std::placeholders::_1));
    main_window->_servoControllerWindow->setControlFunctionCallBack(std::bind(&TuiService::tui_control_function_callback, this, std::placeholders::_1));

    finalcut::FWidget::setMainWidget (main_window);
    Logger::_mainWindow = main_window;
    Logger::_useTui = true;
    main_window->show();

    app->exec();
    Logger::_useTui = false;
    exit(0);

}

void TuiService::printHelp() {
    INFO("Available Commands: ");
    INFO("--> exit : close app");
    INFO("--> openTui");
    INFO("--> startAll : start all services");
    INFO("--> sensors  : get sensor values");
    INFO("--> set <joint> <value>: set servo angle");
    INFO("--> llm <query> : query llm");
    INFO("--> log <className> : print logs of a service");
    INFO("--> gestures <seconds>: prints recognized gesture info while seconds");
    INFO("--> start|stop <service> : start or stop a service");
    INFO("----> Available Services: ");
    INFO("------> WebSocketService");
    INFO("------> VideoStreamService");
    INFO("------> RobotControllerService");
    INFO("------> RemoteConnectionService");
    INFO("------> InteractiveChatService");
    INFO("------> GestureRecognizerService");
    INFO("------> MappingService");
}

void TuiService::setJointAngle(ServoMotorJoint joint, int angle) {

    std::unique_ptr<MessageData> data = std::make_unique<ControlData>();
    data->source = SourceService::tuiService;
    static_cast<ControlData*>(data.get())->jointAngle.emplace();
    static_cast<ControlData*>(data.get())->jointAngle = std::make_pair(joint, angle);

    publish(MessageType::ControlData, data);

}

TuiService::~TuiService() {

}

void TuiService::subcribed_data_receive(MessageType type,  const std::unique_ptr<MessageData>& data) {

    switch (type) {
        case MessageType::LLMResponse:
            if (data) {
                std::lock_guard<std::mutex> lock(_dataMutex);

                std::string response = static_cast<LLMResponseData*>(data.get())->sentence;
                EmotionalGesture emotion = static_cast<LLMResponseData*>(data.get())->emotionalGesture;
                ReactionalGesture reaction = static_cast<LLMResponseData*>(data.get())->reactionalGesture;
                float emotionSimilarity = static_cast<LLMResponseData*>(data.get())->emotionSimilarity;
                float reactionSimilarity = static_cast<LLMResponseData*>(data.get())->reactionSimilarity;

                response += " " + emotion.symbol;
                response += " similarity: " + std::to_string(emotionSimilarity);
                response +=  " " + reaction.symbol;
                response += " similarity: " + std::to_string(reactionSimilarity) + " ";

                _tuiLlmResponseCallBackFunction(response);
            }
            break;

        case MessageType::SensorData:
            if (data) {
                std::lock_guard<std::mutex> lock(_dataMutex);

                _currentSensorData = *static_cast<SensorData*>(data.get());
                if (_tuiSensorCallBackFunction) {
                    _tuiSensorCallBackFunction(_currentSensorData);
                }

                if (_currentSensorData.compassData.has_value()) {
                    if (_tuiSensorCallBackFunction)
                        _tuiCompasDirectionCallBackFunction(_currentSensorData.compassData->angle);
                }

                if (_currentSensorData.currentJointAngles.has_value()) {
                    if (_tuiMotorFeedBackInfoCallBackFunction)
                        _tuiMotorFeedBackInfoCallBackFunction(_currentSensorData.currentJointAngles.value());
                }

            }
            break;

        case MessageType::RecognizedGesture : {
            if (data) {
                std::lock_guard<std::mutex> lock(_dataMutex);

                _recognizedGestureData = *static_cast<RecognizedGestureData*>(data.get());
            }
            break;
        }
        default:
            WARNING("{} subcribed_data_receive unknown message type!", get_service_name());
            break;
    }
}

void TuiService::tui_control_function_callback(const ControlData &data) {
    std::unique_ptr<MessageData> message_data = std::make_unique<ControlData>();
    *static_cast<ControlData*>(message_data.get()) = data;
    publish(MessageType::ControlData, message_data);
}

void TuiService::tui_llm_query_callback(const std::string &query) {
    std::unique_ptr<MessageData> data = std::make_unique<LLMQueryData>();
    static_cast<LLMQueryData*>(data.get())->query = query;
    publish(MessageType::LLMQuery, data);
}

