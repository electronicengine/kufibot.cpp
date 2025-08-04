#ifndef LANDMARK_TRACKING_SERVICE_H
#define LANDMARK_TRACKING_SERVICE_H

#include "service.h"

class LandmarkTrackingService: public Service{


public:
    static LandmarkTrackingService *get_instance();

    virtual ~LandmarkTrackingService();

private:
    LLMResponseData _llmResponseData;
    RecognizedGestureData _recognizedGestureData;

    static LandmarkTrackingService *_instance;

    LandmarkTrackingService();
    void service_function();
    void searchTheFace();
    //subscribed recognized gesture data
    void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data);

};



#endif //LANDMARK_TRACKING_SERVICE_H
