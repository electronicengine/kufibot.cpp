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

#include "video_stream_service.h"

#include "../logger.h"
#include "gesture_performer_service.h"
#include "interactive_chat_service.h"
#include "landmark_tracker_service.h"
#include "rag_service.h"

VideoStreamService * VideoStreamService::_instance = nullptr;


VideoStreamService *VideoStreamService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new VideoStreamService();
    }
    return _instance;
}


VideoStreamService::VideoStreamService(int cameraIndex) : Service("VideoStreamService"), _cameraIndex(0) {

}


bool VideoStreamService::initialize() {
    _cap = std::make_unique<cv::VideoCapture>(_cameraIndex, cv::CAP_V4L2);
    if (!_cap->isOpened()) {
        ERROR("Error: Could not open the camera.");
        return false;
    }

    _cap->set(cv::CAP_PROP_FRAME_WIDTH, 640);
    _cap->set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    subscribe_to_service(InteractiveChatService::get_instance());
    subscribe_to_service(LandmarkTrackerService::get_instance());
    subscribe_to_service(GesturePerformerService::get_instance());
    subscribe_to_service(RagService::get_instance());

    return true;
}

VideoStreamService::~VideoStreamService() {
    INFO("VideoStreamService destruct...");

    stop();

}

void VideoStreamService::service_function() {

    cv::Mat frame;
    if (!_cap->isOpened()) {
        _cap = std::make_unique<cv::VideoCapture>(_cameraIndex, cv::CAP_V4L2);
        _cap->set(cv::CAP_PROP_FRAME_WIDTH, 640);
        _cap->set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    }

    while (_running) {

        *_cap >> frame;
        if (frame.empty()) {
            WARNING("Warning: Received empty frame!");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        std::unique_ptr<MessageData> data = std::make_unique<VideoFrameData>();
        static_cast<VideoFrameData *>(data.get())->frame = frame;
        publish(MessageType::VideoFrame, data);
    }
    INFO("cap releasing...");
    _cap->release();
    INFO("cap released");
}


void VideoStreamService::subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData> &data) {

    switch (type) {
        case MessageType::InteractiveChatStarted: {
            stop();
            break;
        }

        case MessageType::GesturePerformanceCompleted: {
            start();
            break;
        }

        case MessageType::StopVideoStreamRequest: {
            stop();
            break;
        }

        case MessageType::StartVideoStreamRequest: {
            start();
            break;
        }

        default:
            break;
    }
}
