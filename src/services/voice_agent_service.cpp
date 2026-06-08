#include "voice_agent_service.h"
#include "../logger.h"
#include "../../include/json.hpp"
#include "../public_data_messages.h"

using Json = nlohmann::json;

VoiceAgentService* VoiceAgentService::_instance = nullptr;

VoiceAgentService* VoiceAgentService::get_instance() {
    if (_instance == nullptr) {
        _instance = new VoiceAgentService();
    }
    return _instance;
}

VoiceAgentService::VoiceAgentService() : Service("VoiceAgentService") {
}

VoiceAgentService::~VoiceAgentService() {
}

bool VoiceAgentService::initialize() {
    if (!_udpServer.bindSocket(_listenPort)) {
        ERROR("VoiceAgentService failed to bind to port {}", _listenPort);
        return false;
    }
    INFO("VoiceAgentService initialized on port {}", _listenPort);
    return true;
}

void VoiceAgentService::service_function() {
    INFO("VoiceAgentService thread started.");
    while (_running) {
        std::string receivedData = _udpServer.receiveData(100); // 100ms timeout
        if (!receivedData.empty()) {
            process_received_data(receivedData);
        }
    }
    INFO("VoiceAgentService thread stopping.");
}

void VoiceAgentService::process_received_data(const std::string &data) {
    try {
        Json packet = Json::parse(data);
        if (!packet.contains("type") || !packet.contains("payload")) {
            return;
        }

        MessageType type = static_cast<MessageType>(packet["type"].get<int>());
        std::string payload = packet["payload"];

        INFO("VoiceAgentService received message type: {}", static_cast<int>(type));

        switch (type) {
            case MessageType::SensorReadRequest: {

                publish(::MessageType::SensorReadRequest);
                this_thread::sleep_for(std::chrono::milliseconds(50)); // Wait for sensor data to be published
                _udpServer.sendData(Json{{"type", static_cast<int>(MessageType::SensorData)}, {"payload", _sensorData->to_json()}}.dump());
                break;
            }
            case MessageType::LLMResponse: {

                auto llmresponse = std::make_unique<LLMResponseData>(payload);
                INFO("VoiceAgentService received LLMResponse: {}", llmresponse->to_json());
                publish(MessageType::LLMResponse, std::move(llmresponse));

                break;
            }
            // Add more cases as needed
            default:
                DEBUG("VoiceAgentService: Unhandled message type {}", static_cast<int>(type));
                break;
        }
    } catch (const std::exception &e) {
        ERROR("VoiceAgentService error processing data: {}", e.what());
    }
}


void VoiceAgentService::subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData> &data) {

    switch (type) {

        case MessageType::SensorData:
            if (data) {
                std::lock_guard<std::mutex> lock(_dataMutex);

                if (const auto* sensor = dynamic_cast<const SensorData*>(data.get())) {
                    _sensorData = std::make_shared<SensorData>(*sensor); // copy into shared ownership
                }

            }
            break;
        default:
            break;
    }

}
