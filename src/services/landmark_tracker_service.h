#ifndef LANDMARK_TRACKING_SERVICE_H
#define LANDMARK_TRACKING_SERVICE_H

#include "service.h"

#define LEFT            180
#define RIGHT           0
#define DOWN            270
#define UP              90


enum class TrackState {
    idle,
    tracking,
    engaging_reaction,
};

struct PolarVector {
    int angle, magnitude;
};

struct Point2D {
    int x, y;
};

struct TrackingData {
    RecognizedGestureData recognizedGestureData;
    std::map<ServoMotorJoint, uint8_t> currentJointAngles;

    Point2D getFaceCenter() const {
        Point2D faceCenter{0, 0};

        int sumX = 0;
        int sumY = 0;
        int eyePointCount = 0;

        for (auto landmark : recognizedGestureData.faceLandmarks) {
            // eye points
            if (landmark.id == 33 || landmark.id == 133 || landmark.id == 362 || landmark.id == 263) {
                sumX += landmark.cx;
                sumY += landmark.cy;
                eyePointCount++;
            }
        }

        if (eyePointCount > 0) {
            faceCenter.x = sumX / eyePointCount;
            faceCenter.y = sumY / eyePointCount;
        }

        return faceCenter;
    }

    Point2D getHandCenter() const {
        auto &handBox = recognizedGestureData.handBbox;
        Point2D handCenter{0, 0};
        handCenter.x = (handBox.xmin + handBox.xmax) / 2;
        handCenter.y = (handBox.ymin + handBox.ymax) / 2;

        return handCenter;
    }

};

struct LastKnownTarget {
    std::optional<Point2D> _target;
    int followTimeOut;

    void setValue(Point2D target) {
        followTimeOut = 5;
        _target = target;
    }

    Point2D getValue() {
        if (_target.has_value()) {
            if (--followTimeOut <= 0) {
                _target.reset();
                return Point2D{0, 0};
            }else {
                return _target.value();
            }
        } else {
            return Point2D{0, 0};
        }
    }

};

class LandmarkTrackerService: public Service{

    public:
        static LandmarkTrackerService *get_instance();

        virtual ~LandmarkTrackerService();

    private:
        LLMResponseData _llmResponseData;
        RecognizedGestureData _recognizedGestureData;
        SensorData _sensorData;

        LastKnownTarget _lastKnownTarget;

        static LandmarkTrackerService *_instance;
        std::map<ServoMotorJoint, std::map<GestureJointState, GestureJointAngle>> _jointPositionList;
        int _errorTreshold;
        int _reactionEngageTimeout;

        LandmarkTrackerService();

        void initialize();

        TrackingData collectTrackingData();
        Point2D selectTheTarget(const TrackingData &trackData);
        TrackState getTrackingState(const PolarVector& errorVector);
        int calculateControlMagnitude(const PolarVector& errorVector);

        void service_function();
        void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data);

        void searchFace();
        PolarVector calculateErrorVector(const Point2D &target);

        void controlHead(int angle, int magnitude);
        void engageReaction(TrackingData trackingData);

};



#endif //LANDMARK_TRACKING_SERVICE_H
