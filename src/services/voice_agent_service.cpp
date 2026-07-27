#include "voice_agent_service.h"

#include "../../include/json.hpp"
#include "../logger.h"
#include "../public_data_messages.h"
#include "remote_connection_service.h"

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

    subscribe_to_service(RobotControllerService::get_instance());

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
            INFO("received {} size of data", receivedData.size());

            process_received_data(receivedData);
        }
        publish(MessageType::SensorReadRequest);
    }
    INFO("VoiceAgentService thread stopping.");
}

void VoiceAgentService::process_received_data(const std::string &data) {
    try {
        Json packet = Json::parse(data);
        if (!packet.contains("type") || !packet.contains("payload")) {
            WARNING("undefined received message type: ", packet.dump());

            return;
        }

        MessageType type = static_cast<MessageType>(packet["type"].get<int>());
        std::string payload = packet["payload"];

        INFO("VoiceAgentService received message type: {}", static_cast<int>(type));

        switch (type) {
            case MessageType::SensorReadRequest: {
                if (_sensorData) {
                    Json packet;
                    packet["type"] = static_cast<int>(MessageType::SensorData);
                    packet["payload"] = _sensorData->to_json();
                    INFO("VoiceAgentService sending sensor data: {}", packet.dump());
                    _udpServer.sendData(packet.dump());
                } else {
                    ERROR("VoiceAgentService received SensorReadRequest but no sensor data available");
                }

                break;
            }
            case MessageType::LLMResponse: {
                auto llmresponse = std::make_unique<LLMResponseData>(payload);
                INFO("VoiceAgentService received LLMResponse: {}", llmresponse->to_json());
                publish(MessageType::LLMResponse, std::move(llmresponse));

                break;
            }

            case MessageType::CameraSnapShotRequest: {
                publish(MessageType::CameraSnapShotRequest);
                INFO("VoiceAgentService received CameraImageRequest");
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // wait for image data to be published

                Json packet;
                Json imageData;

                packet["type"] = static_cast<int>(MessageType::CameraSnapShotResponse);
                imageData["snapShotPath"] = _cameraSnapShot;
                packet["payload"] = imageData.dump();

                INFO("VoiceAgentService sending camera snapshot: {}", _cameraSnapShot);
                _udpServer.sendData(packet.dump());
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

        case MessageType::CameraSnapShotResponse: {
            if (data) {
                std::lock_guard<std::mutex> lock(_dataMutex);
                if (const auto* path = dynamic_cast<const CameraSnapShotResponseData*>(data.get())) {
                    _cameraSnapShot = path->imagePath;
                }
            }

        }
        default:
            break;
    }

}
