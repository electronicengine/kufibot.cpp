#ifndef HAND_GESTURE_RECOGNIZING_OPERATOR_H
#define HAND_GESTURE_RECOGNIZING_OPERATOR_H


#include "mediapipe.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include "../public_data_messages.h"




struct DistanceResult {
    double length;
    cv::Mat frame;
    std::vector<int> info; // [x1, y1, x2, y2, cx, cy]
};

class HandGestureRecognizingOperator {
    public:
        HandGestureRecognizingOperator(bool mode = false, int maxHands = 2, float detectionCon = 0.5f, float trackCon = 0.5f);

        ~HandGestureRecognizingOperator();

        bool initialize();

        void cleanup();

        cv::Mat findFingers(cv::Mat& frame, bool draw = false);

        std::pair<std::vector<Landmark>, BoundingBox> findPosition(cv::Mat& frame,  int handNo = 0, bool draw = false);

        std::vector<int> findFingerUp();

        DistanceResult findDistance(int p1, int p2, cv::Mat& frame, bool draw = false, int r = 15, int t = 3);

        std::string detectGesture();

        const std::vector<Landmark>& getLandmarks() const {
            return lmsList;
        }

        const BoundingBox& getBoundingBox() const {
            return bbox;
        }

        int getHandCount() const {
            return results ? results->length : 0;
        }

    private:
        bool mode;
        int maxHands;
        float detectionCon;
        float trackCon;

        mp_instance* instance;
        mp_poller* landmarks_poller;
        mp_poller* handedness_poller;

        mp_multi_face_landmark_list* results;
        std::vector<Landmark> lmsList;
        BoundingBox bbox;

        std::vector<int> tipIds;

        bool initialized;

        // Yardımcı fonksiyonlar
        void clearResults();
};

#endif // HAND_GESTURE_RECOGNIZING_OPERATOR_H