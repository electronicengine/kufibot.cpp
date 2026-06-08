#ifndef VIDEO_OPERATOR_H
#define VIDEO_OPERATOR_H

#include <memory>
#include <mutex>

#include <opencv2/opencv.hpp>

#include "operator.h"

class VideoOperator : public Operator {
public:
    explicit VideoOperator(int cameraIndex = 0, int frameWidth = 640, int frameHeight = 480);
    ~VideoOperator() override;

    bool initialize() override;
    void shutdown() override;
    bool isReady() const noexcept override;

    bool open();
    void close();
    bool readFrame(cv::Mat& frame);

    int getCameraIndex() const noexcept;
    int getFrameWidth() const noexcept;
    int getFrameHeight() const noexcept;
    std::shared_ptr<cv::VideoCapture> getCapture() const;

private:
    bool openLocked();

    int _cameraIndex;
    int _frameWidth;
    int _frameHeight;

    mutable std::mutex _capMutex;
    std::shared_ptr<cv::VideoCapture> _cap;
};

#endif // VIDEO_OPERATOR_H

