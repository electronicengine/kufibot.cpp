#ifndef MAPPING_SERVICE_H
#define MAPPING_SERVICE_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <iostream>
#include <cmath>

#include "service.h"
#include "../controllers/distance_controller.h"  
#include "../controllers/compass_controller.h"

class DistanceController; 
class CompassController;


class MappingService : public Service{
private:
    int _mapWidth, _mapHeight;
    int _centerX, _centerY;
    int _maxMagnitude;
    cv::Mat _polarPlot;
    static MappingService *_instance;


    int _index = 0;

    cv::Point polarToCartesian(double angle, int magnitude) const;
    void updatePlot(double angle, int magnitude);

public:
    MappingService();

    static MappingService *get_instance();
    void service_update_function();
    void start(); 
    void stop();



};

#endif // MAPPING_SERVICE_H