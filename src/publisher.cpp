#include "publisher.h"
#include <opencv2/opencv.hpp>
#include "services/service.h"
#include "logger.h"

void Publisher::subscribe(Subscriber* subscriber) {
    std::lock_guard<std::mutex> lock(_mutex);
    // TRACE("subscribing.. publisher: {} subscriber: {}. subscriber count: {}", _publisherName, ((Service*)subscriber)->get_service_name() , _subscribers.size());
    _subscribers.push_back(subscriber);
}

void Publisher::un_subscribe(Subscriber* subscriber) {
    std::lock_guard<std::mutex> lock(_mutex);
    // TRACE("un_subscribing...publisher: {} subscriber: {}. subscriber count: {}", _publisherName, ((Service*)subscriber)->get_service_name() , _subscribers.size());
    _subscribers.erase(
        std::remove_if(_subscribers.begin(), _subscribers.end(),
            [subscriber](Subscriber* existing_subscriber) {
                return existing_subscriber == subscriber; 
            }),
        _subscribers.end()
    );
}

void Publisher::publish(MessageType type, const std::unique_ptr<MessageData>& data) {
    std::lock_guard<std::mutex> lock(_mutex);
    // TRACE("publishing... publisher: {} subscriber count: {}", _publisherName , _subscribers.size());
    for (const auto& sub : _subscribers) {
        if (sub) {
            if (static_cast<Service*>(sub)->is_running())
                sub->subcribed_data_receive(type, data);
        }
    }
}

