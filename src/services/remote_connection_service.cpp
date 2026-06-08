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

#include "remote_connection_service.h"

#include "../logger.h"
#include "expression_service.h"
#include "interactive_chat_service.h"
#include "perception_service.h"

RemoteConnectionService* RemoteConnectionService::_instance = nullptr;


RemoteConnectionService *RemoteConnectionService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new RemoteConnectionService();
    }
    return _instance;
}

RemoteConnectionService::RemoteConnectionService()
    : Service("RemoteConnectionService"), _port(8080), _ip("0.0.0.0"), _webSocketOperator(_ip, static_cast<uint16_t>(_port)) {

}

bool RemoteConnectionService::initialize() {
    _webSocketOperator.setOpenHandler([this](const ConnectionHandle& hdl) {
        handle_websocket_open(hdl);
    });
    _webSocketOperator.setCloseHandler([this](const ConnectionHandle& hdl) {
        handle_websocket_close(hdl);
    });
    _webSocketOperator.setMessageHandler([this](const ConnectionHandle& hdl, const std::string& message) {
        handle_websocket_message(hdl, message);
    });

    if (!_webSocketOperator.initialize()) {
        ERROR("Web socket operator failed to initialize.");
        return false;
    }

    return true;
}

void RemoteConnectionService::service_function() {
    if (!_webSocketOperator.start()) {
        ERROR("Web socket operator failed to start.");
        _running = false;
        return;
    }

    while (_running) {
        {
            std::unique_lock<std::mutex> lock(_dataMutex);
            _condVar.wait(lock, [this] {
                return !_running || _socketMessage.has_value() || _frame.has_value() || _sensorData.has_value();
            });
        }

        if (!_running) {
            break;
        }

        if (_socketMessage.has_value() && _hdl.has_value()) {
            if (_socketMessage.value() == "on_open") {
                INFO("on_open");

                speak("Uzaktan kumanda modu aktif!");
                std::this_thread::sleep_for(std::chrono::milliseconds(3000));

                INFO("Publishing AIModeOffCall");
                publish(MessageType::AIModeOffCall);
                subscribe_to_service(PerceptionService::get_instance());
                subscribe_to_service(RobotControllerService::get_instance());
            } else if (_socketMessage.value() == "on_close") {
                INFO("on_close");
                INFO("Publishing AIModeOnCall");
                publish(MessageType::AIModeOnCall);
                unsubscribe_from_service(PerceptionService::get_instance());
                unsubscribe_from_service(RobotControllerService::get_instance());
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                speak("EY AY Modu Aktif!");
                _frame.reset();
                _hdl.reset();

            } else {
                Json message = Json::parse(_socketMessage.value());
                if (message.contains("talkie")) {
                    std::string talkieValue = message["talkie"];
                    INFO("Talkie Message: {} ", talkieValue);

                    if (talkieValue.find("switch ai mode on") != std::string::npos) {

                        INFO("Publishing AIModeOnCall");
                        publish(MessageType::AIModeOnCall);

                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                        speak("EY AY Modu aktif!");

                    } else if (talkieValue.find("switch ai mode off") != std::string::npos) {
                        INFO("Publishing AIModeOffCall");

                        speak("Uzaktan kumanda modu aktif!");
                        std::this_thread::sleep_for(std::chrono::milliseconds(3000));

                        publish(MessageType::AIModeOffCall);
                    } else {
                        auto data = std::make_unique<LLMQueryData>();
                        data->query = talkieValue;

                        publish(MessageType::LLMQuery, std::move(data));
                    }
                }else if (message.contains("wifi_config")) {
                        std::string ssid = message["wifi_config"]["SSID"];
                        std::string password = message["wifi_config"]["Password"];
                        INFO("Connecting Wifi: SSID: {} PASSWORD: {} ", ssid, password);

                        std::string cmd = "sudo nmcli device wifi connect ";
                        cmd += ssid;
                        cmd += " password ";
                        cmd += password;
                        cmd += " ifname wlan0";

                        system("sudo nmcli connection down Hotspot");
                        system(cmd.c_str());

                }else if (message.contains("database_insert")) {
                    auto data = std::make_unique<DatabaseInsertData>();
                    data->input = message["database_insert"]["input"];
                    data->output = message["database_insert"]["output"];

                    publish(MessageType::DatabaseInsertData, std::move(data));

                }else {
                    auto data = std::make_unique<ControlData>(message);
                    data->source = SourceService::remoteConnectionService;

                    publish(MessageType::ControlData, std::move(data));
                }
            }

            _socketMessage.reset();
        }

        if (_frame.has_value() && _hdl.has_value()) {
            std::vector<uchar> buffer;
            cv::imencode(".jpg", _frame.value(), buffer);
            _webSocketOperator.sendBinary(_hdl.value(), buffer);
            _frame.reset();

            std::string sensor_json;
            if (_sensorData.has_value()) {
                sensor_json = _sensorData.value().to_json();
            }

            _webSocketOperator.sendText(_hdl.value(), sensor_json);
            _sensorData.reset();
        }
    }

    _webSocketOperator.stop();
}

void RemoteConnectionService::speak(std::string text) {
    INFO("Speaking: {}", text);
    auto data = std::make_unique<SpeakRequestData>();
    data->text = text;
    publish(MessageType::SpeakRequest, std::move(data));
}

RemoteConnectionService::~RemoteConnectionService()
{
    stop();
    _webSocketOperator.shutdown();
}

void RemoteConnectionService::handle_websocket_open(const ConnectionHandle& hdl) {
    std::lock_guard<std::mutex> lock(_dataMutex);
    _socketMessage = "on_open";
    _hdl = hdl;
    _condVar.notify_one();
}

void RemoteConnectionService::handle_websocket_close(const ConnectionHandle& hdl) {
    std::lock_guard<std::mutex> lock(_dataMutex);
    _socketMessage = "on_close";
    _hdl = hdl;
    _condVar.notify_one();
}

void RemoteConnectionService::handle_websocket_message(const ConnectionHandle& hdl, const std::string& message) {
    std::lock_guard<std::mutex> lock(_dataMutex);
    _socketMessage = message;
    _hdl = hdl;
    _condVar.notify_one();
}

void RemoteConnectionService::subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data) {

    switch (type) {
        case MessageType::VideoFrame: {
            if (data) {
                std::lock_guard<std::mutex> lock(_dataMutex);
                if (const auto* frameData = dynamic_cast<const VideoFrameData*>(data.get())) {
                    _frame = frameData->frame;
                    _condVar.notify_one();
                }
            }

            break;
        }

        case MessageType::SensorData: {
            if (data) {
                std::lock_guard<std::mutex> lock(_dataMutex);
                if (const auto* sensorData = dynamic_cast<const SensorData*>(data.get())) {
                    _sensorData = *sensorData;
                }
            }

            break;
        }
        default:
            break;


    }

}
