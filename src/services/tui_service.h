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

    static TuiService *get_instance();

private:
    static TuiService* _instance;

    std::function<void(const SensorData&)> _tuiSensorCallBackFunction;
    std::function<void(const std::string&)> _tuiLlmResponseCallBackFunction;
    std::function<void(const int&)> _tuiCompasDirectionCallBackFunction;
    std::function<void(const std::map<ServoMotorJoint, uint8_t>&)> _tuiMotorFeedBackInfoCallBackFunction;
    SensorData _currentSensorData;
    RecognizedGestureData _recognizedGestureData;

    TuiService();

    void service_function();

    //subscribed sensor_data, llm_response
    void printGestureData(int seconds);
    void openTui();
    void printHelp();
    void setJointAngle(ServoMotorJoint joint, int angle);
    //subscribed sensor_data, llm_response
    void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data);
    void tui_control_function_callback(const ControlData& data);
    void tui_llm_query_callback(const std::string& query);


};

#endif // TUI_SERVICE_H