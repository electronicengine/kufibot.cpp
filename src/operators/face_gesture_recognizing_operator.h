#ifndef FACE_GESTURE_RECOGNIZING_OPERATOR_H
#define FACE_GESTURE_RECOGNIZING_OPERATOR_H

#include "mediapipe.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <cmath>
#include <map>
#include <algorithm>
#include <deque>
#include <iostream>
#include "../public_data_messages.h"



struct CalibrationData {
    double ear;
    double mar;
    double eyebrow;
    double smile;
    double eyebrow_dist;
};

struct BaselineValues {
    double ear;
    double mar;
    double eyebrow;
    double smile;
    double eyebrow_dist;
    bool initialized;

    BaselineValues() : ear(0), mar(0), eyebrow(0), smile(0), eyebrow_dist(0), initialized(false) {}
};

class FaceGestureRecognizingOperator {
private:
    float min_detection_confidence;
    float min_tracking_confidence;

    mp_instance* face_mesh_instance;
    mp_instance* face_detection_instance;

    mp_poller* face_mesh_poller;
    mp_poller* face_detection_poller;

    mp_multi_face_landmark_list* mesh_results;
    std::vector<Landmark> landmarks;

    std::vector<int> LEFT_EYE;
    std::vector<int> RIGHT_EYE;
    std::vector<int> MOUTH;
    std::vector<int> EYEBROW_LEFT;
    std::vector<int> EYEBROW_RIGHT;

    std::string previous_emotion;
    std::deque<std::string> emotion_history;
    int history_size;

    std::vector<CalibrationData> calibration_data;
    int calibration_frames;
    BaselineValues baseline;

    bool initialized;

    void clearResults();
    void initializeLandmarkIndices();

public:
    FaceGestureRecognizingOperator(float min_detection_confidence = 0.5f, float min_tracking_confidence = 0.5f);


    ~FaceGestureRecognizingOperator();

    bool initialize();

    void cleanup();

    std::vector<Landmark> getFaceLandmarks(cv::Mat& frame);
    double calculateDistance(int point1_idx, int point2_idx) const;
    double getEyeAspectRatio(const std::vector<int>& eye_points) const;
    double getMouthAspectRatio() const ;
    double getEyebrowPosition() const;
    std::string detectEmotion();
    FaceInfo getFaceInfo() const;
    void drawLandmarks(cv::Mat& frame, bool draw_all = false) const;
    const std::vector<Landmark>& getLandmarks() const {
        return landmarks;
    }

    std::string getPreviousEmotion() const {
        return previous_emotion;
    }

    bool isCalibrated() const {
        return baseline.initialized;
    }

    void resetCalibration() {
        baseline.initialized = false;
        calibration_frames = 0;
        calibration_data.clear();
        emotion_history.clear();
    }
};
#endif // FACE_GESTURE_RECOGNIZING_OPERATOR_H