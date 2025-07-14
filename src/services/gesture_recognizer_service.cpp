// gesture_recognizer_service.cpp
#include "gesture_recognizer_service.h"
#include <iostream>
#include <algorithm>

GestureRecognizerService::GestureRecognizerService(const std::string& name, const std::string& venvPath)
    : Service(name), venvPath(venvPath), faceRecognizer(venvPath), handRecognizer(venvPath),
      showFaceMesh(false), showFaceInfo(false), showHandLandmarks(true) {
    std::cout << "GestureRecognizerService created: " << name << std::endl;
}

GestureRecognizerService::~GestureRecognizerService() {
    stop();
    if (cap.isOpened()) {
        cap.release();
    }
    cv::destroyAllWindows();
    std::cout << "GestureRecognizerService destroyed: " << _name << std::endl;
}

bool GestureRecognizerService::initialize() {
    std::cout << "Initializing Face Gesture Recognizer..." << std::endl;
    if (!faceRecognizer.initialize()) {
        std::cerr << "Python face gesture recognition module failed to initialize!" << std::endl;
        return false;
    }
    std::cout << "Face Gesture Recognizer Initialized." << std::endl;

    std::cout << "Initializing Hand Gesture Recognizer..." << std::endl;
    if (!handRecognizer.initialize()) {
        std::cerr << "Python hand gesture recognition module failed to initialize!" << std::endl;
        return false;
    }
    std::cout << "Hand Gesture Recognizer Initialized." << std::endl;

    // Kamera başlatma
    cap.open(0, cv::CAP_V4L2);
    if (!cap.isOpened()) {
        std::cerr << "Camera could not be opened! Please check camera connection." << std::endl;
        return false;
    }

    // Kamera çözünürlüğünü ayarla
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    std::cout << "Combined Gesture Recognition started..." << std::endl;
    std::cout << "Press ESC to exit." << std::endl;
    std::cout << "Keys:" << std::endl;
    std::cout << "  'm' - Show/Hide Face Landmarks or full mesh" << std::endl;
    std::cout << "  'i' - Show/Hide detailed face metrics (EAR, MAR)" << std::endl;
    std::cout << "  'h' - Show/Hide Hand Landmarks and Bounding Box" << std::endl;

    return true;
}

void GestureRecognizerService::service_update_function() {
    cv::Mat frame;
    double ptime = 0;

    while (_running) {
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Frame not read, ending stream." << std::endl;
            break;
        }

        processFrame(frame);
        displayFPS(frame, ptime);

        cv::imshow("Integrated Gesture Recognizer", frame);

        int key = cv::waitKey(1) & 0xFF;
        if (key == 27) { // ESC tuşu
            stop();
            break;
        }
        handleKeyPress(key);
    }
}

void GestureRecognizerService::processFrame(cv::Mat& frame) {
    // --- YÜZ İŞLEME ---
    std::string faceEmotion;
    std::vector<int> faceLandmarks;
    std::string faceInfoStr;

    if (faceRecognizer.processFrame(frame, faceEmotion, faceLandmarks, faceInfoStr)) {
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
    std::string handGesture;
    std::vector<int> handLandmarks, handBbox;

    if (handRecognizer.processFrame(frame, handGesture, handLandmarks, handBbox)) {
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
    }
}

std::map<std::string, float> GestureRecognizerService::parseFaceInfoString(const std::string& faceInfoStr) {
    std::map<std::string, float> infoMap;
    std::string cleanedStr = faceInfoStr;

    // Parantezleri ve tek tırnakları kaldır
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
                std::cerr << "Error parsing value '" << valueStr << "': " << e.what() << std::endl;
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

void GestureRecognizerService::handleKeyPress(int key) {
    if (key == 'm' || key == 'M') { // 'm' veya 'M' tuşu (Yüz mesh/noktaları)
        showFaceMesh = !showFaceMesh;
        std::cout << "Face Mesh/Key Points view: " << (showFaceMesh ? "ON" : "OFF") << std::endl;
    } else if (key == 'i' || key == 'I') { // 'i' veya 'I' tuşu (Yüz metrikleri)
        showFaceInfo = !showFaceInfo;
        std::cout << "Detailed Face Info view: " << (showFaceInfo ? "ON" : "OFF") << std::endl;
    } else if (key == 'h' || key == 'H') { // 'h' veya 'H' tuşu (El landmarkları/bbox)
        showHandLandmarks = !showHandLandmarks;
        std::cout << "Hand Landmarks/BBox view: " << (showHandLandmarks ? "ON" : "OFF") << std::endl;
    }
}