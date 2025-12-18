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

#include "face_gesture_recognizing_operator.h"

#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#include "../logger.h"

void FaceGestureRecognizingOperator::clearResults()
{
    if (mesh_results) {
        mp_destroy_multi_face_landmarks(mesh_results);
        mesh_results = nullptr;
    }
}

void FaceGestureRecognizingOperator::initializeLandmarkIndices()
{

    // Sol göz
    LEFT_EYE = {33, 7, 163, 144, 145, 153, 154, 155, 133, 173, 157, 158, 159, 160, 161, 246};

    // Sağ göz
    RIGHT_EYE = {362, 382, 381, 380, 374, 373, 390, 249, 263, 466, 388, 387, 386, 385, 384, 398};

    // Ağız
    MOUTH = {78, 191, 80, 81, 82, 13, 312, 311, 310, 415, 308, 324, 318, 402, 317, 14, 87, 178, 88, 95};

    // Sol kaş
    EYEBROW_LEFT = {70, 63, 105, 66, 107, 55, 65, 52, 53, 46};

    // Sağ kaş
    EYEBROW_RIGHT = {296, 334, 293, 300, 276, 283, 282, 295, 285, 336};

}

FaceGestureRecognizingOperator::FaceGestureRecognizingOperator(float min_detection_confidence, float min_tracking_confidence) :
    min_detection_confidence(min_detection_confidence),
    min_tracking_confidence(min_tracking_confidence),
    face_mesh_instance(nullptr),
    face_detection_instance(nullptr),
    face_mesh_poller(nullptr),
    face_detection_poller(nullptr),
    mesh_results(nullptr),
    previous_emotion("Neutral"),
    history_size(10),
    calibration_frames(0),
    initialized(false) {

    initializeLandmarkIndices();
    initialize();
}

FaceGestureRecognizingOperator::~FaceGestureRecognizingOperator()
{

    cleanup();

}

bool FaceGestureRecognizingOperator::initialize()
{

    if (initialized) return true;

    // Face Mesh instance'ını oluştur
    std::string mesh_path = "/usr/local/bin/mediapipe/modules/face_landmark/face_landmark_front_cpu.binarypb";
    mp_instance_builder* mesh_builder = mp_create_instance_builder(mesh_path.c_str(), "image");

    if (!mesh_builder) {
        ERROR("Failed to create face mesh builder");
        return false;
    }

    // Face Mesh parametreleri
    mp_add_side_packet(mesh_builder, "num_faces", mp_create_packet_int(1));
    mp_add_side_packet(mesh_builder, "with_attention", mp_create_packet_bool(true));
    mp_add_option_float(mesh_builder, "facelandmarkcpu__TensorsToDetectionsCalculator",
                        "min_score_thresh", min_detection_confidence);

    face_mesh_instance = mp_create_instance(mesh_builder);
    if (!face_mesh_instance) {
        ERROR("Failed to create face mesh instance");
        return false;
    }

    // Face Mesh poller
    face_mesh_poller = mp_create_poller(face_mesh_instance, "multi_face_landmarks");
    if (!face_mesh_poller) {
        ERROR("Failed to create face mesh poller");
        return false;
    }

    // Grafiği başlat
    if (!mp_start(face_mesh_instance)) {
        ERROR("Failed to start face mesh graph");
        return false;
    }

    initialized = true;
    return true;

}

void FaceGestureRecognizingOperator::cleanup()
{

    clearResults();

    if (face_mesh_poller) {
        mp_destroy_poller(face_mesh_poller);
        face_mesh_poller = nullptr;
    }

    if (face_detection_poller) {
        mp_destroy_poller(face_detection_poller);
        face_detection_poller = nullptr;
    }

    if (face_mesh_instance) {
        mp_destroy_instance(face_mesh_instance);
        face_mesh_instance = nullptr;
    }

    if (face_detection_instance) {
        mp_destroy_instance(face_detection_instance);
        face_detection_instance = nullptr;
    }

    initialized = false;

}

std::vector<Landmark> FaceGestureRecognizingOperator::getFaceLandmarks(cv::Mat &frame)
{

    if (!initialized) {
        ERROR("Detector not initialized!");
        return {};
    }

    clearResults();
    landmarks.clear();

    // RGB'ye çevir
    cv::Mat rgb_frame;
    cv::cvtColor(frame, rgb_frame, cv::COLOR_BGR2RGB);

    // MediaPipe için image yapısı
    mp_image image;
    image.data = rgb_frame.data;
    image.width = rgb_frame.cols;
    image.height = rgb_frame.rows;
    image.format = mp_image_format_srgb;

    // İşle
    if (!mp_process(face_mesh_instance, mp_create_packet_image(image))) {
        const char* err = mp_get_last_error();
        ERROR("Failed to process frame: {}", (err ? err : "Unknown error"));
        if (err) mp_free_error(err);  // Bellek sızıntısını önle
        return {};
    }

    if (!mp_wait_until_idle(face_mesh_instance)) {
        ERROR("Failed to wait for processing");
        return {};
    }

    // Sonuçları al
    if (mp_get_queue_size(face_mesh_poller) > 0) {
        mp_packet* packet = mp_poll_packet(face_mesh_poller);
        mesh_results = mp_get_norm_multi_face_landmarks(packet);
        mp_destroy_packet(packet);

        if (mesh_results && mesh_results->length > 0) {
            const mp_landmark_list& face = mesh_results->elements[0];

            int h = frame.rows;
            int w = frame.cols;

            for (int id = 0; id < face.length; id++) {
                const mp_landmark& lm = face.elements[id];
                int cx = static_cast<int>(lm.x * w);
                int cy = static_cast<int>(lm.y * h);
                landmarks.push_back({id, cx, cy});
            }
        }
    }

    return landmarks;

}

double FaceGestureRecognizingOperator::getEyebrowPosition() const {
    if (landmarks.empty()) return 0;

    // Sol kaş ve göz arası mesafe
    double left_eyebrow_height = calculateDistance(70, 159);

    // Sağ kaş ve göz arası mesafe
    double right_eyebrow_height = calculateDistance(296, 386);

    double avg_eyebrow_height = (left_eyebrow_height + right_eyebrow_height) / 2.0;
    return avg_eyebrow_height;
}

std::string FaceGestureRecognizingOperator::detectEmotion()
{
    if (landmarks.empty()) {
        return "No Face";
    }

    // Metrikleri hesapla
    double left_ear = getEyeAspectRatio({33, 160, 158, 133, 153, 144});
    double right_ear = getEyeAspectRatio({362, 385, 387, 263, 373, 380});
    double avg_ear = (left_ear + right_ear) / 2.0;
    double mar = getMouthAspectRatio();

    double left_eyebrow_to_eye_dist = calculateDistance(159, 10);
    double right_eyebrow_to_eye_dist = calculateDistance(386, 338);
    double avg_eyebrow_height = (left_eyebrow_to_eye_dist + right_eyebrow_to_eye_dist) / 2.0;

    // Gülümseme yoğunluğu
    double mouth_corner_left_y = landmarks.size() > 308 ? landmarks[308].cy : 0;
    double mouth_corner_right_y = landmarks.size() > 78 ? landmarks[78].cy : 0;
    double nose_tip_y = landmarks.size() > 1 ? landmarks[1].cy : 0;

    double smile_intensity = 0;
    if (mouth_corner_left_y && mouth_corner_right_y) {
        double mouth_corner_avg_y = (mouth_corner_left_y + mouth_corner_right_y) / 2.0;
        smile_intensity = nose_tip_y - mouth_corner_avg_y;
    }

    // Kaş arası mesafe
    double eyebrow_inner_distance = calculateDistance(296, 66);
    double face_width = calculateDistance(234, 454);
    double normalized_eyebrow_distance = face_width > 0 ? eyebrow_inner_distance / face_width : 0;

    // KALİBRASYON
    if (!baseline.initialized) {
        if (calibration_frames < 30) {
            CalibrationData data;
            data.ear = avg_ear;
            data.mar = mar;
            data.eyebrow = avg_eyebrow_height;
            data.smile = smile_intensity;
            data.eyebrow_dist = normalized_eyebrow_distance;

            calibration_data.push_back(data);
            calibration_frames++;

            return "Calibrating... (" + std::to_string(calibration_frames) + "/30)";
        }

        if (calibration_frames == 30) {
            // Ortalama hesapla
            baseline.ear = 0;
            baseline.mar = 0;
            baseline.eyebrow = 0;
            baseline.smile = 0;
            baseline.eyebrow_dist = 0;

            for (const auto& data : calibration_data) {
                baseline.ear += data.ear;
                baseline.mar += data.mar;
                baseline.eyebrow += data.eyebrow;
                baseline.smile += data.smile;
                baseline.eyebrow_dist += data.eyebrow_dist;
            }

            baseline.ear /= calibration_data.size();
            baseline.mar /= calibration_data.size();
            baseline.eyebrow /= calibration_data.size();
            baseline.smile /= calibration_data.size();
            baseline.eyebrow_dist /= calibration_data.size();
            baseline.initialized = true;

            calibration_frames++;

            INFO( "✅ Calibration complete:");
            INFO( "  EAR: {}", baseline.ear);
            INFO( "  MAR: {}", baseline.mar);
            INFO( "  Eyebrow: {}", baseline.eyebrow);
            INFO( "  Smile: {}", baseline.smile);
        }
    }

    // Normalize edilmiş değerler
    double ear_norm = baseline.ear > 0 ? avg_ear / baseline.ear : 1.0;
    double mar_norm = baseline.mar > 0 ? mar / baseline.mar : 1.0;
    double smile_norm = smile_intensity - baseline.smile;
    double eyebrow_norm = baseline.eyebrow > 0 ? avg_eyebrow_height / baseline.eyebrow : 1.0;
    double eyebrow_dist_norm = baseline.eyebrow_dist > 0 ? normalized_eyebrow_distance / baseline.eyebrow_dist : 1.0;

    // DUYGU KURALLARI
    std::string emotion = "normal";

    if (smile_norm > 6) {
        emotion = "mutlu";
    } else if (mar_norm > 120) {
        emotion = "korkmuş";
    } else if (mar_norm > 20) {
        emotion = "şaşkın";
    } else if (smile_norm < -3) {
        emotion = "üzgün";
    } else if (ear_norm < 0.8) {
        emotion = "uykulu";
    } else if (eyebrow_dist_norm < 0.85 && ear_norm < 0.9) {
        emotion = "sinirli";
    }

    // Emotion geçmişi ile yumuşatma
    emotion_history.push_back(emotion);
    if (emotion_history.size() > history_size) {
        emotion_history.pop_front();
    }

    if (emotion_history.size() >= 3) {
        // En sık görülen duyguyu bul
        std::map<std::string, int> counts;
        for (const auto& e : emotion_history) {
            counts[e]++;
        }

        int max_count = 0;
        std::string most_common;
        for (const auto& pair : counts) {
            if (pair.second > max_count) {
                max_count = pair.second;
                most_common = pair.first;
            }
        }

        emotion = most_common;
    }

    previous_emotion = emotion;
    return emotion;
}

FaceInfo FaceGestureRecognizingOperator::getFaceInfo() const
{
    FaceInfo info;

    if (landmarks.empty()) {
        return info;
    }

    info.left_ear = getEyeAspectRatio({33, 160, 158, 133, 153, 144});
    info.right_ear = getEyeAspectRatio({362, 385, 387, 263, 373, 380});
    info.avg_ear = (info.left_ear + info.right_ear) / 2.0;
    info.mar = getMouthAspectRatio();
    info.eyebrow_height = getEyebrowPosition();

    return info;
}

void FaceGestureRecognizingOperator::drawLandmarks(cv::Mat &frame, bool draw_all) const
{
    if (landmarks.empty()) return;

    if (draw_all) {
        // Tüm landmark'ları çiz
        for (const auto& lm : landmarks) {
            cv::circle(frame, cv::Point(lm.cx, lm.cy), 1, cv::Scalar(0, 255, 0), -1);
        }
    } else {
        // Sadece önemli bölgeleri çiz

        // Sol göz
        for (int idx : LEFT_EYE) {
            if (idx < landmarks.size()) {
                cv::circle(frame, cv::Point(landmarks[idx].cx, landmarks[idx].cy),
                            2, cv::Scalar(255, 0, 0), -1);
            }
        }

        // Sağ göz
        for (int idx : RIGHT_EYE) {
            if (idx < landmarks.size()) {
                cv::circle(frame, cv::Point(landmarks[idx].cx, landmarks[idx].cy),
                            2, cv::Scalar(255, 0, 0), -1);
            }
        }

        // Ağız
        for (int idx : MOUTH) {
            if (idx < landmarks.size()) {
                cv::circle(frame, cv::Point(landmarks[idx].cx, landmarks[idx].cy),
                            2, cv::Scalar(0, 0, 255), -1);
            }
        }


        for (int idx : EYEBROW_LEFT) {
            if (idx < landmarks.size()) {
                cv::circle(frame, cv::Point(landmarks[idx].cx, landmarks[idx].cy),
                            2, cv::Scalar(0, 255, 255), -1);
            }
        }

        for (int idx : EYEBROW_RIGHT) {
            if (idx < landmarks.size()) {
                cv::circle(frame, cv::Point(landmarks[idx].cx, landmarks[idx].cy),
                            2, cv::Scalar(0, 255, 255), -1);
            }
        }
    }
}

double FaceGestureRecognizingOperator::getMouthAspectRatio() const {
    if (landmarks.empty()) return 0;

    // Ağız dikey mesafesi
    double vertical_1 = calculateDistance(13, 14);

    // Ağız yatay mesafesi
    double horizontal = calculateDistance(78, 308);

    if (horizontal == 0) return 0;

    double mar = vertical_1 / horizontal;
    return mar;
}

double FaceGestureRecognizingOperator::getEyeAspectRatio(const std::vector<int> &eye_points) const {
    if (landmarks.empty() || eye_points.size() < 6) {
        return 0;
    }

    // Dikey mesafeler
    double vertical_1 = calculateDistance(eye_points[1], eye_points[5]);
    double vertical_2 = calculateDistance(eye_points[2], eye_points[4]);

    // Yatay mesafe
    double horizontal = calculateDistance(eye_points[0], eye_points[3]);

    if (horizontal == 0) return 0;

    double ear = (vertical_1 + vertical_2) / (2.0 * horizontal);
    return ear;
}

double FaceGestureRecognizingOperator::calculateDistance(int point1_idx, int point2_idx) const {
    if (landmarks.empty() || point1_idx >= landmarks.size() || point2_idx >= landmarks.size()) {
        return 0;
    }

    int x1 = landmarks[point1_idx].cx;
    int y1 = landmarks[point1_idx].cy;
    int x2 = landmarks[point2_idx].cx;
    int y2 = landmarks[point2_idx].cy;

    return std::hypot(x2 - x1, y2 - y1);
}