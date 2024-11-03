#include "mapping_service.h"



MappingService* MappingService::_instance = nullptr;

MappingService *MappingService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new MappingService();
    }
    return _instance;
}


MappingService::MappingService(): Service("MappingService") {

}


void MappingService::go_to_point(int targetX, int targetY)
{
    int dx = targetX - _centerX;
    int dy = _centerY - targetY;  


    int targetDistance = static_cast<int>(sqrt(dx * dx + dy * dy));

    double targetAngle = atan2(dy, dx) * 180.0 / CV_PI;

    CompassController* compass = CompassController::get_instance();
    DistanceController* distance = DistanceController::get_instance();
    DCMotorController* motor = DCMotorController::get_instance();

    double currentAngle = compass->get_angle();
    double angleDiff = targetAngle - currentAngle;

    if (angleDiff > 180) angleDiff -= 360;
    if (angleDiff < -180) angleDiff += 360;

    if (angleDiff > 0) {
        motor->turn_right(static_cast<int>(fabs(angleDiff)));
    } else {
        motor->turn_left(static_cast<int>(fabs(angleDiff)));
    }

    std::map<std::string, int> distanceMap = distance->get_distance();
    int currentDistance = distanceMap["distance"];

    while (currentDistance < targetDistance) {
        motor->move_forward(50);  // Adjust speed as needed
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        distanceMap = distance->get_distance();
        currentDistance = distanceMap["distance"];
    }

    motor->stop();
}

void MappingService::service_update_function()
{

    _mapWidth = 1000;
    _mapHeight = 1000; 
    _maxMagnitude = 800;
    _centerX = _mapWidth / 2;
    _centerY = _mapHeight / 2;
    const int unitVectorLength = 100;  

    _polarPlot = cv::Mat::zeros(_mapWidth, _mapWidth, CV_8UC3);
    std::vector<double> angles(640, 0.0);  
    std::vector<int> distances(640, 0);     

    DistanceController* distance = DistanceController::get_instance();
    CompassController* compass = CompassController::get_instance();
    // DCMotorController *motor = DCMotorController::get_instance();


    // motor->turn_right(50);

    cv::namedWindow("Angle Plot", cv::WINDOW_AUTOSIZE);

    for(int i=0; i< 300; i++) {
        double angle = compass->get_angle();
        std::map<std::string, int> distanceMap = distance->get_distance();
        int magnitude = distanceMap["distance"];

        std::cout << "Angle: " << angle << " degrees, Distance: " << magnitude << std::endl;

        angles[_index] = angle;
        distances[_index] = magnitude;
        _index = (_index + 1) % angles.size();  


        cv::Mat resizedPolarPlot;

        cv::Point point = polarToCartesian(angle, magnitude);
        cv::circle(_polarPlot, point, 2, cv::Scalar(0, 255, 0), -1);  // Draw small green dot
        double rad = angle * CV_PI / 180.0;  // Convert angle to radians

        int unitEndX = _centerX + static_cast<int>(unitVectorLength * cos(rad));
        int unitEndY = _centerY - static_cast<int>(unitVectorLength * sin(rad));

        cv::line(_polarPlot, cv::Point(_centerX, _centerY), cv::Point(unitEndX, unitEndY), cv::Scalar(255, 0 , 0), 2);

        cv::circle(_polarPlot, cv::Point(_centerX, _centerY), 5, cv::Scalar(255, 0, 0), -1); // Blue color, filled circle
        cv::resize(_polarPlot, resizedPolarPlot, cv::Size(640, 480));
        cv::imshow("Real-Time Polar Map", resizedPolarPlot);
        // cv::imshow("Angle Plot", graph);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (cv::waitKey(1) == 'q') {
            break;
        }
    }
    // motor->stop();

    // cv::destroyAllWindows();
}

// Converts polar coordinates (angle, magnitude) to Cartesian (x, y)
cv::Point MappingService::polarToCartesian(double angle, int magnitude) const {
    double rad = angle * CV_PI / 180.0;  // Convert angle to radians
    int x = static_cast<int>(_centerX + magnitude * cos(rad));
    int y = static_cast<int>(_centerY - magnitude * sin(rad));  // Invert Y for OpenCV
    return cv::Point(x, y);
}

// Updates the polar plot with a new point
void MappingService::updatePlot(double angle, int magnitude) {

}

void MappingService::start() {
    if (!_running) { 
        _running = true;
        std::cout << "RobotControllerService is starting..." << std::endl;
        _serviceThread = std::thread(&MappingService::service_update_function, this);
    }
}

void MappingService::stop()
{
    if (_running){
        _running = false;
        std::cout << "RobotControllerService is stopping..." << std::endl;
        if (_serviceThread.joinable()) {
            _serviceThread.join(); 
        }
        std::cout << "RobotControllerService is stopped." << std::endl;
    }
}