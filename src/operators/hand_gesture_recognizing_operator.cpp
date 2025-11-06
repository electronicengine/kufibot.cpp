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

#include "hand_gesture_recognizing_operator.h"
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include "../logger.h"


void HandGestureRecognizingOperator::clearResults()
{

    if (results) {
        mp_destroy_multi_face_landmarks(results);
        results = nullptr;
    }
}

HandGestureRecognizingOperator::HandGestureRecognizingOperator(bool mode, int maxHands, float detectionCon, float trackCon)
    : mode(mode), maxHands(maxHands), detectionCon(detectionCon),
      trackCon(trackCon), instance(nullptr), landmarks_poller(nullptr),
      handedness_poller(nullptr), results(nullptr), initialized(false)
{

    // Parmak ucu ID'leri
    tipIds = {4, 8, 12, 16, 20};

    // MediaPipe instance'ını oluştur
    initialize();
}

HandGestureRecognizingOperator::~HandGestureRecognizingOperator()
{
    cleanup();
}

bool HandGestureRecognizingOperator::initialize()
{

    if (initialized) return true;

    std::string path = "/usr/local/bin/mediapipe/modules/hand_landmark/hand_landmark_tracking_cpu.binarypb";
    mp_instance_builder* builder = mp_create_instance_builder(path.c_str(), "image");

    if (!builder) {
        ERROR("Failed to create instance builder");
        return false;
    }

    mp_add_option_float(builder, "palmdetectioncpu__TensorsToDetectionsCalculator",
                        "min_score_thresh", detectionCon);
    mp_add_option_double(builder, "handlandmarkcpu__ThresholdingCalculator",
                        "threshold", trackCon);

    mp_add_side_packet(builder, "num_hands", mp_create_packet_int(maxHands));
    mp_add_side_packet(builder, "model_complexity", mp_create_packet_int(1));
    mp_add_side_packet(builder, "use_prev_landmarks", mp_create_packet_bool(!mode));

    instance = mp_create_instance(builder);
    if (!instance) {
        ERROR( "Failed to create instance");
        return false;
    }

    landmarks_poller = mp_create_poller(instance, "multi_hand_landmarks");
    if (!landmarks_poller) {
        ERROR( "Failed to create landmarks poller");
        return false;
    }

    handedness_poller = mp_create_poller(instance, "multi_handedness");
    if (!handedness_poller) {
        ERROR("Failed to create handedness poller");
        return false;
    }

    // Grafiği başlat
    if (!mp_start(instance)) {
        ERROR("Failed to start graph");
        return false;
    }

    initialized = true;
    return true;

}

void HandGestureRecognizingOperator::cleanup()
{

    clearResults();

    if (handedness_poller) {
        mp_destroy_poller(handedness_poller);
        handedness_poller = nullptr;
    }

    if (landmarks_poller) {
        mp_destroy_poller(landmarks_poller);
        landmarks_poller = nullptr;
    }

    if (instance) {
        mp_destroy_instance(instance);
        instance = nullptr;
    }

    initialized = false;

}

cv::Mat HandGestureRecognizingOperator::findFingers(cv::Mat &frame, bool draw)
{
    if (!initialized) {
        ERROR("Detector not initialized!");
        return frame;
    }

    clearResults();
    lmsList.clear();
    bbox = BoundingBox();

    cv::Mat imgRGB;
    cv::cvtColor(frame, imgRGB, cv::COLOR_BGR2RGB);

    mp_image image;
    image.data = imgRGB.data;
    image.width = imgRGB.cols;
    image.height = imgRGB.rows;
    image.format = mp_image_format_srgb;

    if (!mp_process(instance, mp_create_packet_image(image))) {
        ERROR("Failed to process frame");
        return frame;
    }

    if (!mp_wait_until_idle(instance)) {
        ERROR("Failed to wait for processing");
        return frame;
    }

    if (mp_get_queue_size(landmarks_poller) > 0) {
        mp_packet* landmarks_packet = mp_poll_packet(landmarks_poller);
        results = mp_get_norm_multi_face_landmarks(landmarks_packet);
        mp_destroy_packet(landmarks_packet);
    }

    return frame;
}

std::pair<std::vector<Landmark>, BoundingBox> HandGestureRecognizingOperator::findPosition(cv::Mat &frame, int handNo, bool draw)
{

    lmsList.clear();
    bbox = BoundingBox();

    if (!results || results->length == 0) {
        return {lmsList, bbox};
    }

    if (handNo >= results->length) {
        return {lmsList, bbox};
    }

    const mp_landmark_list& hand = results->elements[handNo];

    std::vector<int> xList, yList;

    for (int id = 0; id < hand.length; id++) {
        const mp_landmark& lm = hand.elements[id];

        int h = frame.rows;
        int w = frame.cols;

        int cx = static_cast<int>(lm.x * w);
        int cy = static_cast<int>(lm.y * h);

        xList.push_back(cx);
        yList.push_back(cy);

        lmsList.push_back({id, cx, cy});

        if (draw) {
            cv::circle(frame, cv::Point(cx, cy), 5, cv::Scalar(255, 0, 255), cv::FILLED);
        }
    }

    if (!xList.empty() && !yList.empty()) {
        int xmin = *std::min_element(xList.begin(), xList.end());
        int xmax = *std::max_element(xList.begin(), xList.end());
        int ymin = *std::min_element(yList.begin(), yList.end());
        int ymax = *std::max_element(yList.begin(), yList.end());

        bbox = BoundingBox(xmin, ymin, xmax, ymax);

        if (draw) {
            cv::rectangle(frame, cv::Point(xmin - 20, ymin - 20),
                        cv::Point(xmax + 20, ymax + 20),
                        cv::Scalar(0, 255, 0), 2);
        }
    }

    return {lmsList, bbox};
}


std::vector<int> HandGestureRecognizingOperator::findFingerUp()
{

    std::vector<int> fingers;

    if (lmsList.empty()) {
        return fingers;
    }

    if (lmsList[tipIds[0]].cx > lmsList[tipIds[0] - 1].cx) {
        fingers.push_back(1);
    } else {
        fingers.push_back(0);
    }

    for (int id = 1; id < 5; id++) {
        if (lmsList[tipIds[id]].cy < lmsList[tipIds[id] - 2].cy) {
            fingers.push_back(1);
        } else {
            fingers.push_back(0);
        }
    }

    return fingers;

}

DistanceResult HandGestureRecognizingOperator::findDistance(int p1, int p2, cv::Mat &frame, bool draw, int r, int t)
{

    DistanceResult result;
    result.frame = frame;
    result.length = 0;

    if (lmsList.empty() || p1 >= lmsList.size() || p2 >= lmsList.size()) {
        return result;
    }

    int x1 = lmsList[p1].cx;
    int y1 = lmsList[p1].cy;
    int x2 = lmsList[p2].cx;
    int y2 = lmsList[p2].cy;
    int cx = (x1 + x2) / 2;
    int cy = (y1 + y2) / 2;

    result.length = std::hypot(x2 - x1, y2 - y1);
    result.info = {x1, y1, x2, y2, cx, cy};

    if (draw) {
        cv::line(frame, cv::Point(x1, y1), cv::Point(x2, y2),
                cv::Scalar(255, 0, 255), t);
        cv::circle(frame, cv::Point(x1, y1), r, cv::Scalar(255, 0, 255), cv::FILLED);
        cv::circle(frame, cv::Point(x2, y2), r, cv::Scalar(255, 0, 255), cv::FILLED);
        cv::circle(frame, cv::Point(cx, cy), r, cv::Scalar(0, 0, 255), cv::FILLED);
    }

    return result;
}

std::string HandGestureRecognizingOperator::detectGesture()
{

    if (lmsList.empty()) {
        return "No Hand";
    }

    std::vector<int> fingers = findFingerUp();

    if (fingers.empty()) {
        return "No Hand";
    }

    bool allOpen = true;
    for (int f : fingers) {
        if (f == 0) {
            allOpen = false;
            break;
        }
    }
    if (allOpen) return "Open Hand";

    bool allClosed = true;
    for (int f : fingers) {
        if (f == 1) {
            allClosed = false;
            break;
        }
    }
    if (allClosed) return "Fist";

    if (fingers == std::vector<int>{0, 1, 0, 0, 0}) {
        return "Pointer Finger";
    }

    if (fingers == std::vector<int>{0, 1, 1, 0, 0}) {
        return "Peace Sign";
    }

    if (fingers == std::vector<int>{1, 0, 0, 0, 1}) {
        return "Phone Hand";
    }

    if (fingers == std::vector<int>{0, 0, 0, 0, 1}) {
        return "Pinky Finger";
    }

    if (fingers[1] == 0 && fingers[2] == 0 && fingers[3] == 0 && fingers[4] == 0) {
        cv::Mat dummy = cv::Mat::zeros(1, 1, CV_8UC3);
        auto distResult = findDistance(4, 8, dummy, false);
        if (distResult.length < 50) {
            return "OK Sign";
        }
    }

    return "Unknown Gesture";

}
