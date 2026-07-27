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
#include "robot_controller_service.h"
#include "remote_connection_service.h"
#include "expression_service.h"
#include "mapping_service.h"
#include "interactive_chat_service.h"

#undef K
#undef null

#include <oneapi/tbb/detail/_range_common.h>

#include "../logger.h"
#include "../tui/main_window.h"
#include "../tui/widget_color_theme.h"
#include "final/final.h"
#include "perception_service.h"
#include "rag_service.h"

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
    subscribe_to_service(PerceptionService::get_instance());

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
            RobotControllerService::get_instance()->stop();
            RemoteConnectionService::get_instance()->stop();
            InteractiveChatService::get_instance()->stop();
            ExpressionService::get_instance()->stop();
            PerceptionService::get_instance()->stop();
            MappingService::get_instance()->stop();

            std::exit(0);
        }else if (input =="openTui") {
            openTui();
        }else if (input == "help") {
            printHelp();
        }else if (input == "startAll") {
            RobotControllerService::get_instance()->start();
            RemoteConnectionService::get_instance()->start();
            InteractiveChatService::get_instance()->start();
            ExpressionService::get_instance()->start();
            PerceptionService::get_instance()->start();
            MappingService::get_instance()->start();
        }else if (input.find("start") != std::string::npos) {
            if (input.find("RobotControllerService") != std::string::npos) {
                RobotControllerService::get_instance()->start();
            }else if (input.find("InteractiveChatService") != std::string::npos) {
                InteractiveChatService::get_instance()->start();
            }else if (input.find("RemoteConnectionService") != std::string::npos) {
                RemoteConnectionService::get_instance()->start();
            }else if (input.find("PerceptionService") != std::string::npos) {
                PerceptionService::get_instance()->start();
            }else if (input.find("MappingService") != std::string::npos) {
                MappingService::get_instance()->start();
            }else if (input.find("ExpressionService") != std::string::npos) {
                ExpressionService::get_instance()->start();
            }else if (input.find("RagService") != std::string::npos) {
                RagService::get_instance()->start();
            }
        }else if (input.find("stop") != std::string::npos) {
            if (input.find("RobotControllerService") != std::string::npos) {
                RobotControllerService::get_instance()->stop();
            }else if (input.find("InteractiveChatService") != std::string::npos) {
                InteractiveChatService::get_instance()->stop();
            }else if (input.find("RemoteConnectionService") != std::string::npos) {
                RemoteConnectionService::get_instance()->stop();
            }else if (input.find("PerceptionService") != std::string::npos) {
                PerceptionService::get_instance()->stop();
            }else if (input.find("MappingService") != std::string::npos) {
                MappingService::get_instance()->stop();
            }else if (input.find("ExpressionService") != std::string::npos) {
                ExpressionService::get_instance()->stop();
            }else if (input.find("RagService") != std::string::npos) {
                RagService::get_instance()->stop();
            }
        }else if (input.find("log" ) != std::string::npos) {
            std::string className = input.substr(4, input.size() - 4);
            Logger::print_cached_logs(className);
        }else if(input == "sensors") {
            publish(MessageType::SensorReadRequest);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

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
                    INFO("Joint: {} Angle: {}", joint, angle);
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
        }else if (input.find("rag update") != std::string::npos) {
        publish(MessageType::UpdateRAGDatabaseRequest);
        }else if (input.find("rag clear") != std::string::npos) {
            publish(MessageType::ClearRAGDatabaseRequest);
        }else if (input.find("rag show") != std::string::npos) {
            publish(MessageType::ShowRAGDatabaseRequest);
        }else if (input.find("speak") != std::string::npos) {
            // Extract text after "speak" command
            size_t speakPos = input.find("speak");
            std::string textToSpeak = input.substr(speakPos + 5);

            // Trim leading/trailing whitespace
            size_t start = textToSpeak.find_first_not_of(" \t\"'");
            size_t end = textToSpeak.find_last_not_of(" \t\"'");

            if (start != std::string::npos && end != std::string::npos) {
                textToSpeak = textToSpeak.substr(start, end - start + 1);

                // Create and publish SpeakRequestData
                auto data = std::make_unique<SpeakRequestData>();
                data->text = textToSpeak;
                publish(MessageType::SpeakRequest, std::move(data));
                INFO("Speaking: {}", textToSpeak);
            } else {
                WARNING("speak command usage: speak \"text to speak\" or speak text");
            }
        }else if (input.find("expression") != std::string::npos) {
            std::string cmd = input;
            size_t pos = cmd.find("expression");
            std::string arg = cmd.substr(pos + 10);

            // Trim leading/trailing spaces
            size_t start = arg.find_first_not_of(" \t\r\n\"'");
            size_t end = arg.find_last_not_of(" \t\r\n\"'");
            std::string target = (start != std::string::npos && end != std::string::npos) ? arg.substr(start, end - start + 1) : "";

            if (target.empty() || target == "help") {
                INFO("Usage: expression <gesture_name>");
                INFO("       expression list");
                INFO("Examples: expression happy");
                INFO("          expression greeting");
            } else if (target == "list") {
                INFO("Available Emotional Gestures:");
                INFO("--> happy, angry, funny, serious, curious, worried, surprised, confident");
                INFO("Available Reactional Gestures:");
                INFO("--> greeting, listening, talking, accepting, rejecting, thinking, agreeing");
            } else {
                bool found = false;
                LLMResponseData data;
                data.endMarker = true;
                data.sentence = "Testing " + target;

                if (target == "happy") {
                    data.emotionalGesture.emotion = EmotionType::happy;
                    data.emotionSimilarity = 1.0f;
                    data.reactionSimilarity = 0.0f;
                    found = true;
                } else if (target == "angry") {
                    data.emotionalGesture.emotion = EmotionType::angry;
                    data.emotionSimilarity = 1.0f;
                    data.reactionSimilarity = 0.0f;
                    found = true;
                } else if (target == "funny") {
                    data.emotionalGesture.emotion = EmotionType::funny;
                    data.emotionSimilarity = 1.0f;
                    data.reactionSimilarity = 0.0f;
                    found = true;
                } else if (target == "serious") {
                    data.emotionalGesture.emotion = EmotionType::serious;
                    data.emotionSimilarity = 1.0f;
                    data.reactionSimilarity = 0.0f;
                    found = true;
                } else if (target == "curious") {
                    data.emotionalGesture.emotion = EmotionType::curious;
                    data.emotionSimilarity = 1.0f;
                    data.reactionSimilarity = 0.0f;
                    found = true;
                } else if (target == "worried") {
                    data.emotionalGesture.emotion = EmotionType::worried;
                    data.emotionSimilarity = 1.0f;
                    data.reactionSimilarity = 0.0f;
                    found = true;
                } else if (target == "surprised") {
                    data.emotionalGesture.emotion = EmotionType::surprised;
                    data.emotionSimilarity = 1.0f;
                    data.reactionSimilarity = 0.0f;
                    found = true;
                } else if (target == "confident") {
                    data.emotionalGesture.emotion = EmotionType::confident;
                    data.emotionSimilarity = 1.0f;
                    data.reactionSimilarity = 0.0f;
                    found = true;
                } else if (target == "greeting") {
                    data.reactionalGesture.reaction = ReactionType::greeting;
                    data.reactionSimilarity = 1.0f;
                    data.emotionSimilarity = 0.0f;
                    found = true;
                } else if (target == "listening") {
                    data.reactionalGesture.reaction = ReactionType::listening;
                    data.reactionSimilarity = 1.0f;
                    data.emotionSimilarity = 0.0f;
                    found = true;
                } else if (target == "talking") {
                    data.reactionalGesture.reaction = ReactionType::talking;
                    data.reactionSimilarity = 1.0f;
                    data.emotionSimilarity = 0.0f;
                    found = true;
                } else if (target == "accepting") {
                    data.reactionalGesture.reaction = ReactionType::accepting;
                    data.reactionSimilarity = 1.0f;
                    data.emotionSimilarity = 0.0f;
                    found = true;
                } else if (target == "rejecting") {
                    data.reactionalGesture.reaction = ReactionType::rejecting;
                    data.reactionSimilarity = 1.0f;
                    data.emotionSimilarity = 0.0f;
                    found = true;
                } else if (target == "thinking") {
                    data.reactionalGesture.reaction = ReactionType::thinking;
                    data.reactionSimilarity = 1.0f;
                    data.emotionSimilarity = 0.0f;
                    found = true;
                } else if (target == "agreeing") {
                    data.reactionalGesture.reaction = ReactionType::agreeing;
                    data.reactionSimilarity = 1.0f;
                    data.emotionSimilarity = 0.0f;
                    found = true;
                }

                if (found) {
                    INFO("Publishing LLMResponse with gesture: {}", target);
                    auto msgData = std::make_unique<LLMResponseData>(data);
                    publish(MessageType::LLMResponse, std::move(msgData));
                } else {
                    WARNING("Unknown gesture: '{}'. Type 'expression list' to see all valid gestures.", target);
                }
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

    if (!RobotControllerService::get_instance()->is_running()) {
        RobotControllerService::get_instance()->start();
    }

    int argc = 0;
    char **argv = nullptr;

    FApplication *app;
    MainWindow *main_window;

    app = new FApplication{argc, argv};
    app->setColorTheme<AWidgetColorTheme>();
    main_window = new MainWindow{app};
    main_window->setText ("Log View");
    main_window->setGeometry (FPoint{1, 0}, FSize{FVTerm::getFOutput()->getColumnNumber(), FVTerm::getFOutput()->getLineNumber()});

    _tuiSensorCallBackFunction = [main_window](const SensorData& sensorData) {
        main_window->queue_sensor_data(sensorData);
    };
    _tuiLlmResponseCallBackFunction = [main_window](const std::string& response) {
        main_window->queue_llm_response(response);
    };
    _tuiCompasDirectionCallBackFunction = [main_window](const int& angle) {
        main_window->queue_compass_direction(angle);
    };
    _tuiMotorFeedBackInfoCallBackFunction = [main_window](const std::map<ServoMotorJoint, uint8_t>& jointAngles) {
        main_window->queue_servo_joints(jointAngles);
    };
    main_window->set_sensor_refresh_callback([this]() {
        publish(MessageType::SensorReadRequest);
    });

    //set tui control CallBack Functions
    main_window->_chatWindow->set_llm_query_function_callback(std::bind(&TuiService::tui_llm_query_callback, this, std::placeholders::_1));
    main_window->_bodyControllerWindow->setControlFunctionCallBack(std::bind(&TuiService::tui_control_function_callback, this, std::placeholders::_1));
    main_window->_servoControllerWindow->setControlFunctionCallBack(std::bind(&TuiService::tui_control_function_callback, this, std::placeholders::_1));

    finalcut::FWidget::setMainWidget (main_window);
    Logger::_mainWindow = main_window;
    Logger::_useTui = true;
    main_window->show();
    main_window->_measurementsWindow->show();
    main_window->_bodyControllerWindow->show();
    main_window->_servoControllerWindow->show();

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
    INFO("--> speak <text> : make robot speak");
    INFO("--> log <className> : print logs of a service");
    INFO("--> gestures <seconds>: prints recognized gesture info while seconds");
    INFO("--> expression <name> : test gesture by name (e.g. happy, greeting)");
    INFO("--> expression list : list all testable expressions");
    INFO("--> start|stop <service> : start or stop a service");
    INFO("----> Available Services: ");
    INFO("------> PerceptionService (camera + perception)");
    INFO("------> RobotControllerService");
    INFO("------> RemoteConnectionService");
    INFO("------> InteractiveChatService");
    INFO("------> MappingService");
    INFO("------> ExpressionService");
}

void TuiService::setJointAngle(ServoMotorJoint joint, int angle) {

    auto data = std::make_unique<ControlData>();
    data->source = SourceService::tuiService;
    data->jointAngle.emplace(joint, angle);

    publish(MessageType::ControlData, std::move(data));

}

TuiService::~TuiService() {

}

void TuiService::subcribed_data_receive(MessageType type,  const std::unique_ptr<MessageData>& data) {

    switch (type) {
        case MessageType::LLMResponse:
            if (data) {
                std::lock_guard<std::mutex> lock(_dataMutex);

                const auto* llmResponse = dynamic_cast<const LLMResponseData*>(data.get());
                if (llmResponse == nullptr) {
                    break;
                }

                std::string response = llmResponse->sentence;
                EmotionalGesture emotion = llmResponse->emotionalGesture;
                ReactionalGesture reaction = llmResponse->reactionalGesture;
                float emotionSimilarity = llmResponse->emotionSimilarity;
                float reactionSimilarity = llmResponse->reactionSimilarity;

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

                const auto* sensorData = dynamic_cast<const SensorData*>(data.get());
                if (sensorData == nullptr) {
                    break;
                }

                _currentSensorData = *sensorData;
                if (_tuiSensorCallBackFunction) {
                    _tuiSensorCallBackFunction(_currentSensorData);
                }

                if (_currentSensorData.compassData.has_value()) {
                    if (_tuiCompasDirectionCallBackFunction)
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

                const auto* recognizedGesture = dynamic_cast<const RecognizedGestureData*>(data.get());
                if (recognizedGesture == nullptr) {
                    break;
                }

                _recognizedGestureData = *recognizedGesture;
            }
            break;
        }

        case MessageType::RecognizedSpeech : {
            std::lock_guard<std::mutex> lock(_dataMutex);

            const auto* recognizedSpeech = dynamic_cast<const RecognizedSpeechData*>(data.get());
            if (recognizedSpeech == nullptr) {
                break;
            }

            _recognizedSpeechData = *recognizedSpeech;
            INFO("Recognized Speech: {}", _recognizedSpeechData.text);

            break;
        }

        default:
            break;
    }
}

void TuiService::tui_control_function_callback(const ControlData &data) {
    auto message_data = std::make_unique<ControlData>();
    *message_data = data;
    message_data->source = SourceService::tuiService;
    publish(MessageType::ControlData, std::move(message_data));
}

void TuiService::tui_llm_query_callback(const std::string &query) {
    auto data = std::make_unique<LLMQueryData>();
    data->query = query;
    publish(MessageType::LLMQuery, std::move(data));
}

