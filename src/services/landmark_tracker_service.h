#ifndef LANDMARK_TRACKING_SERVICE_H
#define LANDMARK_TRACKING_SERVICE_H

#include "service.h"

class LandmarkTrackerService: public Service{


public:
    static LandmarkTrackerService *get_instance();

    virtual ~LandmarkTrackerService();

private:
    LLMResponseData _llmResponseData;
    RecognizedGestureData _recognizedGestureData;

    static LandmarkTrackerService *_instance;

    LandmarkTrackerService();
    void service_function();
    void searchTheFace();
    //subscribed recognized gesture data
    void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data);

};



#endif //LANDMARK_TRACKING_SERVICE_H
