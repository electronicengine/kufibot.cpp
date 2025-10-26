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

#include "gesture_recognizer_service.h"
#include "video_stream_service.h"
#include <algorithm>
#include "../logger.h"
#include "../subscriber.h"

GestureRecognizerService* GestureRecognizerService::_instance = nullptr;

GestureRecognizerService *GestureRecognizerService::get_instance( bool showFrame, const std::string& name)
{
    if (_instance == nullptr) {
        _instance = new GestureRecognizerService(name, showFrame);
    }
    return _instance;
}

GestureRecognizerService::GestureRecognizerService(const std::string& name, bool showFrame)
    : Service(name), _showFrame(showFrame) {
}

GestureRecognizerService::~GestureRecognizerService() {
    stop();
    if (cap.isOpened()) {
        cap.release();
    }
    cv::destroyAllWindows();
}

bool GestureRecognizerService::initialize() {

    subscribe_to_service(VideoStreamService::get_instance());

    if (!_faceGestureRecognizingOperator.initialize()) {
        ERROR("Face gesture recognition module failed to initialize!");
        return false;
    }
    INFO("Face gesture recognition module initialized.");

    if (!_handGestureRecognizingOperator.initialize()) {
        ERROR("Hand gesture recognition module failed to initialize!");
        return false;
    }
    INFO("Hand gesture recognition module initialized.");

    return true;
}

void GestureRecognizerService::service_function() {

    initialize();

    while (_running) {
        std::unique_lock<std::mutex> lock(_dataMutex);

        //pop from _frameQueue and process frame
        if (!_frameQueue.empty()) {
            cv::Mat frame = _frameQueue.front();
            _frameQueue.pop();
            lock.unlock();
            processFrame(frame);

            if (_showFrame) {
                cv::imshow("Integrated Gesture Recognizer", frame);
                cv::waitKey(1);
            }

        } else {
            _condVar.wait(lock);
        }
    }
}

void GestureRecognizerService::subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData> &data) {
    switch (type) {
        case MessageType::VideoFrame: {
            if (data) {
                std::unique_lock<std::mutex> lock(_dataMutex);
                cv::Mat frame = static_cast<VideoFrameData *>(data.get())->frame;
                _frameQueue.push(frame);
                _condVar.notify_one();
            }
            break;
        }
        default:
            WARNING("{} subcribed_data_receive unknown message type!", get_service_name());
            break;
    }
}


void GestureRecognizerService::processFrame(cv::Mat& frame) {

    std::unique_ptr<MessageData> data = std::make_unique<RecognizedGestureData>();
    auto &faceLandMarks = static_cast<RecognizedGestureData*>(data.get())->faceLandmarks;
    auto &faceEmotion = static_cast<RecognizedGestureData*>(data.get())->faceEmotion;
    auto &faceInfo = static_cast<RecognizedGestureData*>(data.get())->faceInfo;

    auto &handBox = static_cast<RecognizedGestureData*>(data.get())->handBbox;
    auto &handLandMarks = static_cast<RecognizedGestureData*>(data.get())->handLandmarks;
    auto &handGesture = static_cast<RecognizedGestureData*>(data.get())->handGesture;

    faceLandMarks = _faceGestureRecognizingOperator.getFaceLandmarks(frame);

    if (!faceLandMarks.empty()) {
        faceEmotion = _faceGestureRecognizingOperator.detectEmotion();
        faceInfo = _faceGestureRecognizingOperator.getFaceInfo();
    }
    if (_showFrame) {
        _faceGestureRecognizingOperator.drawLandmarks(frame, true);
    }

    _handGestureRecognizingOperator.findFingers(frame, _showFrame);
    int handCount = _handGestureRecognizingOperator.getHandCount();

    for (int handNo = 0; handNo < handCount; handNo++) {
        auto [LandMarks, Box] = _handGestureRecognizingOperator.findPosition(frame, handNo, _showFrame);
        handLandMarks = LandMarks;
        handBox = Box;
        handGesture = _handGestureRecognizingOperator.detectGesture();
    }

    publish(MessageType::RecognizedGesture, data);

}

std::map<std::string, float> GestureRecognizerService::parseFaceInfoString(const std::string& faceInfoStr) {
    std::map<std::string, float> infoMap;
    std::string cleanedStr = faceInfoStr;

    // remove prantesis and quo's
    cleanedStr.erase(std::remove(cleanedStr.begin(), cleanedStr.end(), '{'), cleanedStr.end());
    cleanedStr.erase(std::remove(cleanedStr.begin(), cleanedStr.end(), '}'), cleanedStr.end());
    cleanedStr.erase(std::remove(cleanedStr.begin(), cleanedStr.end(), '\''), cleanedStr.end());
    cleanedStr.erase(std::remove(cleanedStr.begin(), cleanedStr.end(), ' '), cleanedStr.end());

    std::stringstream ss(cleanedStr);
    std::string segment;
    while(std::getline(ss, segment, ',')) {
        size_t colonPos = segment.find(':');
        if (colonPos != std::string::npos) {
            std::string key = segment.substr(0, colonPos);
            std::string valueStr = segment.substr(colonPos + 1);
            try {
                infoMap[key] = std::stof(valueStr);
            } catch (const std::exception& e) {
                ERROR("Error parsing value: {} - {}", valueStr, e.what());
            }
        }
    }
    return infoMap;
}

void GestureRecognizerService::displayFPS(cv::Mat& frame, double& ptime) {
    double ctime = static_cast<double>(cv::getTickCount()) / cv::getTickFrequency();
    double fps = 1.0 / (ctime - ptime);
    ptime = ctime;
    cv::putText(frame, "FPS: " + std::to_string(static_cast<int>(fps)),
                cv::Point(10, frame.rows - 20), cv::FONT_HERSHEY_SIMPLEX, 0.7,
                cv::Scalar(255, 0, 255), 2, cv::LINE_AA);
}
