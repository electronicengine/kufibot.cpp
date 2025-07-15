//
// Created by ybulb on 7/15/2025.
//

#include "tui_service.h"

TuiService::~TuiService() {
}

TuiService * TuiService::get_instance() {
}


TuiService::TuiService() : Service("TUI") {
}



void TuiService::subcribed_data_receive(MessageType type, MessageData *data) {
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
