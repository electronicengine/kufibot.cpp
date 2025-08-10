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

GestureRecognizerService *GestureRecognizerService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new GestureRecognizerService();
    }
    return _instance;
}

GestureRecognizerService::GestureRecognizerService(const std::string& name)
    : Service(name), showFaceMesh(true), showFaceInfo(true), showHandLandmarks(true) {
    _initialized = false;
}

GestureRecognizerService::~GestureRecognizerService() {
    stop();
    if (cap.isOpened()) {
        cap.release();
    }
    cv::destroyAllWindows();
}


bool GestureRecognizerService::initialize() {
    _faceGestureRecognizingOperator = new FaceGestureRecognizingOperator("/home/kufi/venv");
    _handGestureRecognizingOperator = new HandGestureRecognizingOperator("/home/kufi/venv");

    if (!_faceGestureRecognizingOperator->initialize()) {
        ERROR("Face gesture recognition module failed to initialize!");
        return false;
    }
    INFO("Face gesture recognition module initialized.");

    if (!_handGestureRecognizingOperator->initialize()) {
        ERROR("Hand gesture recognition module failed to initialize!");
        return false;
    }
    INFO("Hand gesture recognition module initialized.");

    return true;
}

void GestureRecognizerService::service_function() {


    subscribe_to_service(VideoStreamService::get_instance());

}

void GestureRecognizerService::subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data) {
    std::lock_guard<std::mutex> lock(_dataMutex);

    switch (type) {
        case MessageType::VideoFrame: {
            if (data) {
                cv::Mat frame = static_cast<VideoFrameData*>(data.get())->frame;
                video_frame(frame);
            }
            break;
        }
        default:
            WARNING("{} subcribed_data_receive unknown message type!", get_service_name());
            break;
    }
}

void GestureRecognizerService::video_frame(cv::Mat &frame) {

    if (!_initialized) {
        if (!initialize()) {
            ERROR("GestureRecognizerService couldn't initialized correctly!");
        }
        _initialized = true;
    }


    if (_running) {
        if (frame.empty()) {
            ERROR("Frame not read, ending stream.");
        }

        processFrame(frame);

        // displayFPS(frame, ptime);
        // cv::imshow("Integrated Gesture Recognizer", frame);
        // cv::waitKey(1); // Allows GUI events to be processed

    }

}

void GestureRecognizerService::processFrame(cv::Mat& frame) {
    // --- YÜZ İŞLEME ---

    std::unique_ptr<MessageData> data = std::make_unique<RecognizedGestureData>();

    std::string &faceEmotion = static_cast<RecognizedGestureData*>(data.get())->faceGesture;
    std::vector<int> &faceLandmarks = static_cast<RecognizedGestureData*>(data.get())->faceLandmark;
    std::string faceInfoStr;

    if (_faceGestureRecognizingOperator->processFrame(frame, faceEmotion, faceLandmarks, faceInfoStr)) {
        // Duygu bilgisini ekrana yaz
        cv::putText(frame, "Emotion: " + faceEmotion, cv::Point(10, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);

        // Detaylı yüz bilgilerini (showFaceInfo açıksa) ekrana yaz
        if (showFaceInfo) {
            std::map<std::string, float> faceInfo = parseFaceInfoString(faceInfoStr);
            int y_offset = 70;
            for (const auto& pair : faceInfo) {
                cv::putText(frame, pair.first + ": " + std::to_string(pair.second),
                            cv::Point(10, y_offset), cv::FONT_HERSHEY_SIMPLEX, 0.5,
                            cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
                y_offset += 25;
            }
        }

        // Yüz Landmarklarını çiz
        if (!faceLandmarks.empty() && showFaceMesh) {
            // Önemli landmark noktalarını vurgula
            for (size_t i = 0; i < faceLandmarks.size(); i += 3) {
                int id = faceLandmarks[i];
                int cx = faceLandmarks[i+1];
                int cy = faceLandmarks[i+2];

                // Göz kenarları
                if (id == 33 || id == 133 || id == 362 || id == 263) {
                    cv::circle(frame, cv::Point(cx, cy), 2, cv::Scalar(0, 255, 0), cv::FILLED);
                }
                // Ağız köşeleri ve dudaklar
                else if (id == 78 || id == 308 || id == 13 || id == 14) {
                    cv::circle(frame, cv::Point(cx, cy), 2, cv::Scalar(0, 0, 255), cv::FILLED);
                }
                // Kaşlar
                else if (id == 70 || id == 105 || id == 296 || id == 334) {
                    cv::circle(frame, cv::Point(cx, cy), 2, cv::Scalar(255, 0, 0), cv::FILLED);
                }
                else {
                    // Diğer tüm landmarkları küçük nokta olarak çiz
                    cv::circle(frame, cv::Point(cx, cy), 1, cv::Scalar(200, 200, 200), cv::FILLED);
                }
            }
        } else if (!faceLandmarks.empty() && !showFaceMesh) {
             // Sadece tüm landmark'ları küçük sarı noktalar olarak çiz
            for (size_t i = 0; i < faceLandmarks.size(); i += 3) {
                int cx = faceLandmarks[i+1];
                int cy = faceLandmarks[i+2];
                cv::circle(frame, cv::Point(cx, cy), 1, cv::Scalar(0, 255, 255), cv::FILLED);
            }
        }
    } else {
        // Python tarafından bir hata oluştuysa
        cv::putText(frame, "Face Detection Failed", cv::Point(10, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
    }

    // --- EL İŞLEME ---
    std::string &handGesture = static_cast<RecognizedGestureData*>(data.get())->handGesture;
    std::vector<int> &handLandmarks = static_cast<RecognizedGestureData*>(data.get())->handLandmark;
    std::vector<int> handBbox;

    if (_handGestureRecognizingOperator->processFrame(frame, handGesture, handLandmarks, handBbox)) {
        if (showHandLandmarks) {
            if (!handBbox.empty() && handBbox.size() == 4) {
                cv::rectangle(frame, cv::Point(handBbox[0] - 20, handBbox[1] - 20), // Bbox'ı biraz büyüt
                              cv::Point(handBbox[2] + 20, handBbox[3] + 20),
                              cv::Scalar(255, 0, 255), 2); // Mor renk
                cv::putText(frame, handGesture, cv::Point(handBbox[0], handBbox[1] - 30),
                            cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(255, 0, 255), 2); // Mor renk
            }

            for (size_t i = 0; i + 2 < handLandmarks.size(); i += 3) {
                int cx = handLandmarks[i+1], cy = handLandmarks[i+2];
                cv::circle(frame, cv::Point(cx, cy), 5, cv::Scalar(255, 255, 0), cv::FILLED); // Sarı renk
            }
        }
    } else {
        // El algılaması başarısız olursa bir şey çizme veya hata mesajı gösterme
        // cv::putText(frame, "Hand Detection Failed", cv::Point(10, 60),
        //             cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
    }

    publish(MessageType::RecognizedGesture, data);
    // FPS hesaplama ve gösterme
    //double ctime = static_cast<double>(cv::getTickCount()) / cv::getTickFrequency();

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
