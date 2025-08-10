#include "robot_controller_service.h"
#include "gesture_performer_service.h"
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


RobotControllerService::RobotControllerService() : Service("RobotControllerService")
{

}

void RobotControllerService::service_function() {



    subscribe_to_service(MappingService::get_instance());
    subscribe_to_service(GesturePerformerService::get_instance());
    subscribe_to_service(TuiService::get_instance());
    _robot.start();

    while (_running) {
        std::this_thread::sleep_for(std::chrono::microseconds(200));
        if(!_subscribers.empty()){
            std::unique_ptr<MessageData> data = std::make_unique<SensorData>();
            *static_cast<SensorData*>(data.get()) = _robot.get_sensor_values();
            publish(MessageType::SensorData, data);
        }
    }

    _robot.stop();
}

RobotControllerService::~RobotControllerService()
{
}





void RobotControllerService::subcribed_data_receive(MessageType type,  const std::unique_ptr<MessageData>& data) {
    std::lock_guard<std::mutex> lock(_dataMutex);

    switch (type) {
        case MessageType::ControlData: {
            if (data) {
                INFO("RobotControllerService::subcribed_data_receive-ControlData");
                SourceService source;
                ControlData controlData = *static_cast<ControlData*>(data.get());
                if (data->source.has_value()) {
                    source = data->source.value();
                }
                else {
                    source = SourceService::none;
                }

                _robot.postEvent(ControlEvent(EventType::control, source, controlData));

            }
            break;
        }
        default:
            WARNING("{} subcribed_data_receive unknown message type!", get_service_name());
            break;
    }
}


