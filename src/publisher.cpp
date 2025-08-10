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

