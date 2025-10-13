#ifndef LANDMARK_TRACKING_SERVICE_H
#define LANDMARK_TRACKING_SERVICE_H

#include "service.h"

#define LEFT            180
#define RIGHT           0
#define DOWN            270
#define UP              90

struct ErrorVector {
    int angle;
    int magnitude;
};

class LandmarkTrackerService: public Service{

    public:
        static LandmarkTrackerService *get_instance();

        virtual ~LandmarkTrackerService();

    private:
        LLMResponseData _llmResponseData;
        RecognizedGestureData _recognizedGestureData;
        SensorData _sensorData;
        std::atomic<bool> _handFound;
        std::atomic<bool> _faceFound;
        std::atomic<bool> _gesturePerformanceCompleted;

        static LandmarkTrackerService *_instance;
        std::map<ServoMotorJoint, std::map<GestureJointState, GestureJointAngle>> _jointLimits;
        int _errorTreshold;
        int _talkingTimeCounter;

        LandmarkTrackerService();

        void initialize();
        void service_function();
        void searchTheFace();
        //subscribed recognized gesture data
        void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data);

        void searchFace();
        ErrorVector calculateErrorVector(int centerX, int centerY);

        void controlHead(int angle, int magnitude);
        void sayHello();

};



#endif //LANDMARK_TRACKING_SERVICE_H
