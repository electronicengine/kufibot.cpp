// gesture_recognizer_service.cpp
#include "gesture_recognizer_service.h"
#include <iostream>
#include <algorithm>
#include "../logger.h"

GestureRecognizerService::GestureRecognizerService(const std::string& name)
    : Service(name), showFaceMesh(true), showFaceInfo(true), showHandLandmarks(true) {
}

GestureRecognizerService::~GestureRecognizerService() {
    stop();
    if (cap.isOpened()) {
        cap.release();
    }
    cv::destroyAllWindows();
}

bool GestureRecognizerService::initialize() {
    _faceGestureRecognizingOperator = FaceGestureRecognizingOperator::get_instance();
    _handGestureRecognizingOperator = HandGestureRecognizingOperator::get_instance();

    if (!_faceGestureRecognizingOperator->initialize()) {
        Logger::error("Face gesture recognition module failed to initialize!");
        return false;
    }
    Logger::info("Face gesture recognition module initialized.");

    if (!_handGestureRecognizingOperator->initialize()) {
        Logger::error("Hand gesture recognition module failed to initialize!");
        return false;
    }
    Logger::info("Hand gesture recognition module initialized.");

    return true;
}

void GestureRecognizerService::service_update_function() {

}

void GestureRecognizerService::update_video_frame(const cv::Mat &frame) {
    if (_running) {
        static double ptime = 0;
        cv::Mat frame_copy = frame.clone();
        if (frame.empty()) {
            Logger::error("Frame not read, ending stream.");
        }

        processFrame(frame_copy);
        displayFPS(frame_copy, ptime);

        cv::imshow("Integrated Gesture Recognizer", frame_copy);
    }

}

void GestureRecognizerService::processFrame(cv::Mat& frame) {
    // --- Face Processing ---
    std::string faceEmotion;
    std::vector<int> faceLandmarks;
    std::string faceInfoStr;

    if (_faceGestureRecognizingOperator->processFrame(frame, faceEmotion, faceLandmarks, faceInfoStr)) {
        // print emotion info
        cv::putText(frame, "Emotion: " + faceEmotion, cv::Point(10, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);

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

        // Draw Face Landmarks
        if (!faceLandmarks.empty() && showFaceMesh) {
            for (size_t i = 0; i < faceLandmarks.size(); i += 3) {
                int id = faceLandmarks[i];
                int cx = faceLandmarks[i+1];
                int cy = faceLandmarks[i+2];

                // Eye
                if (id == 33 || id == 133 || id == 362 || id == 263) {
                    cv::circle(frame, cv::Point(cx, cy), 2, cv::Scalar(0, 255, 0), cv::FILLED);
                }
                // Mounth
                else if (id == 78 || id == 308 || id == 13 || id == 14) {
                    cv::circle(frame, cv::Point(cx, cy), 2, cv::Scalar(0, 0, 255), cv::FILLED);
                }
                // EyeBrow
                else if (id == 70 || id == 105 || id == 296 || id == 334) {
                    cv::circle(frame, cv::Point(cx, cy), 2, cv::Scalar(255, 0, 0), cv::FILLED);
                }
                else {
                    // other landmarks
                    cv::circle(frame, cv::Point(cx, cy), 1, cv::Scalar(200, 200, 200), cv::FILLED);
                }
            }
        } else if (!faceLandmarks.empty() && !showFaceMesh) {
             //Just draw all landmarks as little yellow dots
            for (size_t i = 0; i < faceLandmarks.size(); i += 3) {
                int cx = faceLandmarks[i+1];
                int cy = faceLandmarks[i+2];
                cv::circle(frame, cv::Point(cx, cy), 1, cv::Scalar(0, 255, 255), cv::FILLED);
            }
        }
    } else {
        cv::putText(frame, "Face Detection Failed", cv::Point(10, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
    }

    // --- Hand Process ---
    std::string handGesture;
    std::vector<int> handLandmarks, handBbox;

    if (_handGestureRecognizingOperator->processFrame(frame, handGesture, handLandmarks, handBbox)) {
        if (showHandLandmarks) {
            if (!handBbox.empty() && handBbox.size() == 4) {
                cv::rectangle(frame, cv::Point(handBbox[0] - 20, handBbox[1] - 20), // Bbox'ı biraz büyüt
                              cv::Point(handBbox[2] + 20, handBbox[3] + 20),
                              cv::Scalar(255, 0, 255), 2); // Mor renk
                cv::putText(frame, handGesture, cv::Point(handBbox[0], handBbox[1] - 30),
                            cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(255, 0, 255), 2); // Purple color
            }

            for (size_t i = 0; i + 2 < handLandmarks.size(); i += 3) {
                int cx = handLandmarks[i+1], cy = handLandmarks[i+2];
                cv::circle(frame, cv::Point(cx, cy), 5, cv::Scalar(255, 255, 0), cv::FILLED); // Yellow color
            }
        }
    }
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
                Logger::error("Error parsing value: {} - {}", valueStr, e.what());
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


void GestureRecognizerService::start()
{
    if (!_running) {
        _running = true;
        Logger::info("GestureRecognizerService is starting...");
        if (!initialize()) {
            Logger::error("GestureRecognizerService couldn't initialized correctly!");
            stop();
        }

        VideoStreamService *video_stream_service = VideoStreamService::get_instance();
        video_stream_service->subscribe(this);

        _serviceThread = std::thread(&GestureRecognizerService::service_update_function, this);
    }
}

void GestureRecognizerService::stop()
{
    if (_running){
        _running = false;
        VideoStreamService *video_stream_service = VideoStreamService::get_instance();
        video_stream_service->un_subscribe(this);

        Logger::info("GestureRecognizerService is stopping...");
        if (_serviceThread.joinable()) {
            _serviceThread.join();
        }
        Logger::info("GestureRecognizerService is stopped.");

    }
}