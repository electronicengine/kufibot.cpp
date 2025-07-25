// VideoStream.h
#ifndef TUI_SERVICE_H
#define TUI_SERVICE_H

#include "service.h"

namespace finalcut {
    class FApplication;
}

class MainWindow;

class TuiService : public Service {

public:
    virtual ~TuiService();

    static TuiService *get_instance(MainWindow* mainWindow = nullptr, finalcut::FApplication *app = nullptr, bool useTui = true);


private:
    static TuiService* _instance;

    std::function<void(const SensorData&)> _tuiSensorCallBackFunction;
    std::function<void(const std::string&)> _tuiLlmResponseCallBackFunction;
    std::function<void(const int&)> _tuiCompasDirectionCallBackFunction;
    std::function<void(const std::map<ServoMotorJoint, uint8_t>&)> _tuiMotorFeedBackInfoCallBackFunction;
    MainWindow* _mainWindow;
    finalcut::FApplication * _app;
    bool _useTui;

    TuiService(MainWindow* mainWindow, finalcut::FApplication *app, bool useTui);

    void service_function();

    //subscribed sensor_data, llm_response
    void sensor_data(Json values);
    void llm_response(const std::string& response);
    void commandLinePrompt();
    //subscribed sensor_data, llm_response
    void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data);
    void tui_control_function_callback(const ControlData& data);
    void tui_llm_query_callback(const std::string& query);


};

#endif // TUI_SERVICE_H