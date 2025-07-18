//
// Created by ybulb on 7/15/2025.
//

#include "tui_service.h"
#include "web_socket_service.h"
#include "robot_controller_service.h"
#include "remote_connection_service.h"
#include "video_stream_service.h"
#include "gesture_performing_service.h"
#include "mapping_service.h"
#include "interactive_chat_service.h"
#include "robot_controller_service.h"

#undef K
#undef null

#include "../logger.h"
#include "../tui/main_window.h"
#include "../tui/widget_color_theme.h"

using namespace finalcut;


TuiService* TuiService::_instance = nullptr;


TuiService *TuiService::get_instance(int argc, char *argv[])
{


    if (_instance == nullptr) {
        _instance = new TuiService(argc, argv);
    }
    return _instance;
}

TuiService::TuiService(int argc, char *argv[]) : Service("TuiService"), _argc(argc), _argv(argv)  {

}


void TuiService::commandLinePrompt() {

    _tuiLlmResponseCallBackFunction = [this](const std::string& response) {
        INFO(response);
    };

    while (1) {
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
            stop();

            std::exit(0);

        }else if(input == "sensors") {
            _tuiSensorCallBackFunction = [this](const SensorData &sensor_data) {
                INFO(sensor_data.to_json().dump());
                _tuiSensorCallBackFunction = nullptr;
            };
        }else {
            tui_llm_query_callback(input);
        }
    }
}

void TuiService::service_function() {

    bool useUI = true;

    for (int i = 1; i < _argc; i++) {
        std::string arg = _argv[i];
        if (arg == "--no-tui") {
            useUI = false;
            break;
        }
    }

    subscribe_to_service(RobotControllerService::get_instance());
    subscribe_to_service(InteractiveChatService::get_instance());

    if (useUI) {
        FApplication app{_argc, _argv};
        app.setColorTheme<AWidgetColorTheme>();
        MainWindow main_dlg {&app};
        main_dlg.setText ("Log View");
        main_dlg.setGeometry (FPoint{1, 0}, FSize{FVTerm::getFOutput()->getColumnNumber(), FVTerm::getFOutput()->getLineNumber()});

        //set tui Prob Callback Functions
        _tuiSensorCallBackFunction = main_dlg._measurementsWindow->get_sensor_data_callback_function();
        _tuiLlmResponseCallBackFunction = main_dlg._chatWindow->get_llm_response_callback_function();
        _tuiCompasDirectionCallBackFunction = main_dlg._compassRTGraphWindow->get_compas_direction_callback_function();
        _tuiServoJointInfoCallBackFunction = main_dlg._servoControllerWindow->get_servo_joints_callback_function();

        //set tui control CallBack Functions
        main_dlg._chatWindow->set_llm_query_function_callback(std::bind(&TuiService::tui_llm_query_callback, this, std::placeholders::_1));
        main_dlg._bodyControllerWindow->setControlFunctionCallBack(std::bind(&TuiService::tui_control_function_callback, this, std::placeholders::_1));
        main_dlg._servoControllerWindow->setControlFunctionCallBack(std::bind(&TuiService::tui_control_function_callback, this, std::placeholders::_1));

        finalcut::FWidget::setMainWidget (&main_dlg);
        main_dlg.show();

        app.exec();
    }else {
        commandLinePrompt();
    }

}

TuiService::~TuiService() {

}

void TuiService::subcribed_data_receive(MessageType type,  const std::unique_ptr<MessageData>& data) {
    std::lock_guard<std::mutex> lock(_dataMutex);

    switch (type) {
        case MessageType::LLMResponse:
            if (data) {
                std::string response = static_cast<LLMResponseData*>(data.get())->sentence;
                EmotionType emotion = static_cast<LLMResponseData*>(data.get())->emotion;
                ReactionType reaction = static_cast<LLMResponseData*>(data.get())->reaction;
                float emotionSimilarity = static_cast<LLMResponseData*>(data.get())->emotionSimilarity;
                float reactionSimilarity = static_cast<LLMResponseData*>(data.get())->reactionSimilarity;

                response += " <" + get_emotion_symbol(emotion) + "> ";
                response += " similarity: " + std::to_string(emotionSimilarity);
                response += " <" + get_reaction_symbol(reaction) + "> ";
                response += " similarity: " + std::to_string(reactionSimilarity);

                _tuiLlmResponseCallBackFunction(response);
            }
            break;

        case MessageType::SensorData:
            if (data) {
                SensorData sensor_data = *static_cast<SensorData*>(data.get());
                if (_tuiSensorCallBackFunction)
                    _tuiSensorCallBackFunction(sensor_data);

                if (sensor_data.compassData.has_value())
                    if (_tuiSensorCallBackFunction)
                        _tuiCompasDirectionCallBackFunction(sensor_data.compassData->angle);

                if (sensor_data.currentJointAngles.has_value())
                    if (_tuiSensorCallBackFunction)
                        _tuiServoJointInfoCallBackFunction(sensor_data.currentJointAngles.value());
            }
            break;
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

