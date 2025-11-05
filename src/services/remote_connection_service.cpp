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
#include "gesture_performer_service.h"
#include "interactive_chat_service.h"
#include "landmark_tracker_service.h"
#include "web_socket_service.h"

RemoteConnectionService* RemoteConnectionService::_instance = nullptr;


RemoteConnectionService *RemoteConnectionService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new RemoteConnectionService();
    }
    return _instance;
}

RemoteConnectionService::RemoteConnectionService() : Service("RemoteConnectionService") {

}

bool RemoteConnectionService::initialize() {
    subscribe_to_service(WebSocketService::get_instance());
    return true;
}

void RemoteConnectionService::service_function() {

    while (_running) {
        {
            std::unique_lock<std::mutex> lock(_dataMutex);
            _condVar.wait(lock);
        }

        if (_frame.has_value() && _hdl.has_value()) {
            std::vector<uchar> buffer;
            cv::imencode(".jpg", _frame.value(), buffer);

            std::unique_ptr<MessageData> data= std::make_unique<WebSocketTransferData>();
            static_cast<WebSocketTransferData*>(data.get())->hdl = _hdl.value();
            static_cast<WebSocketTransferData*>(data.get())->msg = std::string(buffer.begin(), buffer.end());
            static_cast<WebSocketTransferData*>(data.get())->type = 2;
            publish(MessageType::WebSocketTransfer, data);
            _frame.reset();

            std::string sensor_json = "";
            if (_sensorData.has_value()) {
                sensor_json = _sensorData.value().to_json();
            }

            static_cast<WebSocketTransferData*>(data.get())->hdl = _hdl.value();
            static_cast<WebSocketTransferData*>(data.get())->msg = sensor_json;
            static_cast<WebSocketTransferData*>(data.get())->type = 1;
            publish(MessageType::WebSocketTransfer, data);
            _sensorData.reset();
        }

        if (_socketMessage.has_value() && _hdl.has_value()) {
            if(_socketMessage.value() == "on_open"){
                INFO("on_open");
                INFO("Publishing AIModeOffCall");
                publish(MessageType::AIModeOffCall);
                subscribe_to_service(VideoStreamService::get_instance());
                subscribe_to_service(RobotControllerService::get_instance());
            }
            else if(_socketMessage.value() == "on_close"){
                INFO("on_close");
                INFO("Publishing AIModeOffCall");
                publish(MessageType::AIModeOnCall);
                unsubscribe_from_service(VideoStreamService::get_instance());
                unsubscribe_from_service(RobotControllerService::get_instance());
            }
            else{
                Json message = Json::parse(_socketMessage.value());
                if (message.contains("talkie")) {
                    std::string talkieValue = message["talkie"];
                    INFO("Talkie Message: {} ",talkieValue);

                    if (talkieValue.find("switch ai mode on") != std::string::npos) {
                        INFO("Publishing AIModeOnCall");
                        publish(MessageType::AIModeOnCall);
                    }else if (talkieValue.find("switch ai mode off") != std::string::npos) {
                        INFO("Publishing AIModeOffCall");
                        publish(MessageType::AIModeOffCall);
                    }else {
                        std::unique_ptr<MessageData> data = std::make_unique<LLMQueryData>();
                        static_cast<LLMQueryData *>(data.get())->query = talkieValue;
                        publish(MessageType::LLMQuery, data);
                    }

                }else{
                    std::unique_ptr<MessageData> data = std::make_unique<ControlData>();
                    *static_cast<ControlData*>(data.get()) = ControlData(message);
                    data->source = SourceService::remoteConnectionService;

                    publish(MessageType::ControlData, data);
                }
            }

            _socketMessage.reset();
        }
    }
}

RemoteConnectionService::~RemoteConnectionService()
{
    stop(); // Ensure everything is stopped in the destructor
}

void RemoteConnectionService::subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data) {

    switch (type) {
        case MessageType::WebSocketReceive: {
            if (data) {
                std::lock_guard<std::mutex> lock(_dataMutex);
                _socketMessage = static_cast<WebSocketReceiveData*>(data.get())->msg;
                _hdl = static_cast<WebSocketReceiveData*>(data.get())->hdl;
                _condVar.notify_one();
            }
            break;
        }

        case MessageType::VideoFrame: {
            if (data) {
                std::lock_guard<std::mutex> lock(_dataMutex);
                _frame = static_cast<VideoFrameData*>(data.get())->frame;
                _condVar.notify_one();
            }

            break;
        }

        case MessageType::SensorData: {
            if (data) {
                std::lock_guard<std::mutex> lock(_dataMutex);
                _sensorData = *static_cast<SensorData*>(data.get());
            }

            break;
        }
        default:
            break;


    }

}
