#ifndef HAND_GESTURE_RECOGNIZING_OPERATOR_H
#define HAND_GESTURE_RECOGNIZING_OPERATOR_H


#include "mediapipe.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

// Landmark yapısı
struct Landmark {
    int id;
    int cx;
    int cy;
};

// Bounding Box yapısı
struct BoundingBox {
    int xmin;
    int ymin;
    int xmax;
    int ymax;
    bool valid;

    BoundingBox() : xmin(0), ymin(0), xmax(0), ymax(0), valid(false) {}
    BoundingBox(int x1, int y1, int x2, int y2)
        : xmin(x1), ymin(y1), xmax(x2), ymax(y2), valid(true) {}
};

// Python'daki findDistance fonksiyonu
struct DistanceResult {
    double length;
    cv::Mat frame;
    std::vector<int> info; // [x1, y1, x2, y2, cx, cy]
};

class HandGestureRecognizingOperator {
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

public:
    HandGestureRecognizingOperator(bool mode = false, int maxHands = 2, float detectionCon = 0.5f, float trackCon = 0.5f);

    ~HandGestureRecognizingOperator();

    bool initialize();

    void cleanup();

    cv::Mat findFingers(cv::Mat& frame, bool draw = true);

    std::pair<std::vector<Landmark>, BoundingBox> findPosition(cv::Mat& frame,  int handNo = 0, bool draw = true);

    std::vector<int> findFingerUp();

    DistanceResult findDistance(int p1, int p2, cv::Mat& frame, bool draw = true, int r = 15, int t = 3);

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
};

#endif // HAND_GESTURE_RECOGNIZING_OPERATOR_H