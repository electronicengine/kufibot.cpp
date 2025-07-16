//
// Created by ybulb on 7/15/2025.
//

#include "tui_service.h"



TuiService* TuiService::_instance = nullptr;


TuiService *TuiService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new TuiService();
    }
    return _instance;
}

TuiService::TuiService() : Service("TUI") {
}

void TuiService::service_function() {


}

TuiService::~TuiService() {
}

void TuiService::subcribed_data_receive(MessageType type,  const std::unique_ptr<MessageData>& data) {
    std::lock_guard<std::mutex> lock(_dataMutex);

    switch (type) {
        case MessageType::LLMResponse:
            if (data) {
                // query veya response
            }
            break;

        case MessageType::SensorData:
            if (data) {
                // Sensor veya Control JSON
            }
            break;

    }
}
