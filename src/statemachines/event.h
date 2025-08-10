
#ifndef EVENT_H
#define EVENT_H

#include <string>
#include "../public_data_messages.h"


enum class SourceService {
    none,
    gesturePerformerService,
    landmarkTrackerService,
    mappingService,
    tuiControlService,
    remoteControlService,

};

enum EventType {
    none = 0,
    critical_error,
    stop,
    timeout,
    control_head,
    control_wheels,
    control_joints
};

// --------------------------
// Event types
// --------------------------
struct ControlEvent {
    EventType type;
    SourceService source;
    ControlData controlData;

    ControlEvent( EventType _type = none, SourceService _source = SourceService::none, ControlData _data = ControlData()) : type(_type),source(_source), controlData(_data) {}
};


#endif //EVENT_H
