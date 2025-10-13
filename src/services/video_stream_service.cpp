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

VideoStreamService* VideoStreamService::_instance = nullptr;


VideoStreamService *VideoStreamService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new VideoStreamService();
    }
    return _instance;
}


VideoStreamService::VideoStreamService(int cameraIndex) : Service("VideoStreamService"), _cameraIndex(0){
}

VideoStreamService::~VideoStreamService() {
    INFO("VideoStreamService destruct...");

    stop();

}

void VideoStreamService::service_function(){

    cv::Mat frame;
    cv::VideoCapture cap(_cameraIndex, cv::CAP_V4L2);
    if(!cap.isOpened()) {
        ERROR("Error: Could not open the camera.");
        return;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    while (_running ) {

        cap >> frame;
        if (frame.empty()) {
            WARNING("Warning: Received empty frame!");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        std::unique_ptr<MessageData> data = std::make_unique<VideoFrameData>();
        static_cast<VideoFrameData*>(data.get())->frame = frame;
        publish(MessageType::VideoFrame, data);

    }
    INFO("cap releasing...");
    cap.release();
    INFO("cap released");

}
