#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include "public_data_messages.h"

class Subscriber {

public:
    virtual ~Subscriber() = default;

    virtual void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData> &data) = 0;
};

#endif // SUBSCRIBER_H