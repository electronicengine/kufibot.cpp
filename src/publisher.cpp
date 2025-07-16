#include "publisher.h"
#include <opencv2/opencv.hpp>

void Publisher::subscribe(Subscriber* subscriber) {
    std::lock_guard<std::mutex> lock(_mutex);
    _subscribers.push_back(subscriber);
}

void Publisher::un_subscribe(Subscriber* subscriber) {
    std::lock_guard<std::mutex> lock(_mutex);

    _subscribers.erase(
        std::remove_if(_subscribers.begin(), _subscribers.end(),
            [subscriber](Subscriber* existing_subscriber) {
                return existing_subscriber == subscriber; 
            }),
        _subscribers.end()
    );
}

void Publisher::publish(MessageType type, const std::unique_ptr<MessageData>& data) {

    for (const auto& sub : _subscribers) {
        if (sub) {
            sub->subcribed_data_receive(type, data);
        }
    }
}

