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

#include "robot_controller_service.h"
#include "gesture_performer_service.h"
#include "landmark_tracker_service.h"
#include "remote_connection_service.h"
#include "mapping_service.h"
#include "tui_service.h"

#include "../logger.h"

RobotControllerService* RobotControllerService::_instance = nullptr;

RobotControllerService *RobotControllerService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new RobotControllerService();
    }
    return _instance;
}


RobotControllerService::RobotControllerService() : Service("RobotControllerService") {}


bool RobotControllerService::initialize() {

    subscribe_to_service(MappingService::get_instance());
    subscribe_to_service(GesturePerformerService::get_instance());
    subscribe_to_service(TuiService::get_instance());
    subscribe_to_service(InteractiveChatService::get_instance());
    subscribe_to_service(LandmarkTrackerService::get_instance());
    subscribe_to_service(RemoteConnectionService::get_instance());

    _robot.start();

    return true;

}

void RobotControllerService::service_function() {



    while (_running) {
        {
            std::unique_lock<std::mutex> lock(_dataMutex);
            _condVar.wait(lock);
        }

        if (_controlData.has_value()) {
            SourceService source;

            if (_controlData->source.has_value()) {
                source = static_cast<MessageData*>(&_controlData.value())->source.value();
            }
            else {
                source = SourceService::none;
            }

            _robot.postEvent(ControlEvent(EventType::control, source, _controlData.value()));
            _controlData.reset();

            std::this_thread::sleep_for(std::chrono::microseconds(5));
            publishSensorValues();

        }
    }

    _robot.stop();
}


RobotControllerService::~RobotControllerService()
{
}


void RobotControllerService::subcribed_data_receive(MessageType type,  const std::unique_ptr<MessageData>& data) {

    switch (type) {
        case MessageType::ControlData: {
            if (data) {
                {
                    std::lock_guard<std::mutex> lock(_dataMutex);
                    _controlData = *static_cast<ControlData*>(data.get());
                }
            _condVar.notify_one();
            }
            break;
        }

        case MessageType::SensorReadRequest: {
            _sensorReadRequest = true;
        }

        default:
            break;
    }
}

void RobotControllerService::publishSensorValues() {
    std::unique_ptr<MessageData> data = std::make_unique<SensorData>();
    *static_cast<SensorData *>(data.get()) = _robot.getSensorValues();
    publish(MessageType::SensorData, data);
}
