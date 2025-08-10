
#ifndef EVENT_H
#define EVENT_H

#include <string>
#include "../public_data_messages.h"


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
