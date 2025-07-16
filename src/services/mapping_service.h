#ifndef MAPPING_SERVICE_H
#define MAPPING_SERVICE_H

#include "service.h"


class MappingService : public Service{


public:
    static MappingService *get_instance();

    virtual ~MappingService();

private:
    int _mapWidth, _mapHeight;
    int _centerX, _centerY;
    int _maxMagnitude;
    cv::Mat _polarPlot;
    static MappingService *_instance;

    int _index = 0;
    MappingService();
    void service_function();
    void go_to_point(int x, int y);

    cv::Point polarToCartesian(double angle, int magnitude) const;
    void updatePlot(double angle, int magnitude);

    //subscribed sensor_data
    void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data);;

    void sensor_data(Json values);


};

#endif // MAPPING_SERVICE_H