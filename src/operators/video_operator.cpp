#include "video_operator.h"

#include "../logger.h"

namespace {
#ifdef _WIN32
constexpr int VIDEO_CAPTURE_BACKEND = cv::CAP_ANY;
#else
constexpr int VIDEO_CAPTURE_BACKEND = cv::CAP_V4L2;
#endif
}

VideoOperator::VideoOperator(int cameraIndex, int frameWidth, int frameHeight)
    : Operator("VideoOperator"),
      _cameraIndex(cameraIndex),
      _frameWidth(frameWidth),
      _frameHeight(frameHeight) {
}

VideoOperator::~VideoOperator() {
    close();
}

bool VideoOperator::initialize() {
    return open();
}

void VideoOperator::shutdown() {
    close();
}

bool VideoOperator::isReady() const noexcept {
    std::lock_guard<std::mutex> lock(_capMutex);
    return _cap != nullptr && _cap->isOpened();
}

bool VideoOperator::open() {
    std::lock_guard<std::mutex> lock(_capMutex);
    return openLocked();
}

bool VideoOperator::openLocked() {
    if (_cap != nullptr && _cap->isOpened()) {
        return true;
    }

    auto capture = std::make_shared<cv::VideoCapture>(_cameraIndex, VIDEO_CAPTURE_BACKEND);
    if (!capture->isOpened()) {
        ERROR("Error: Could not open the camera with index {}.", _cameraIndex);
        _cap.reset();
        return false;
    }

    capture->set(cv::CAP_PROP_FRAME_WIDTH, _frameWidth);
    capture->set(cv::CAP_PROP_FRAME_HEIGHT, _frameHeight);
    _cap = capture;
    return true;
}

void VideoOperator::close() {
    std::lock_guard<std::mutex> lock(_capMutex);
    if (_cap != nullptr && _cap->isOpened()) {
        INFO("Releasing camera {}", _cameraIndex);
        _cap->release();
    }
    _cap.reset();
}

bool VideoOperator::readFrame(cv::Mat& frame) {
    std::lock_guard<std::mutex> lock(_capMutex);

    if (!openLocked()) {
        return false;
    }

    (*_cap) >> frame;
    if (frame.empty()) {
        WARNING("Warning: Received empty frame from camera {}!", _cameraIndex);
        return false;
    }

    return true;
}

int VideoOperator::getCameraIndex() const noexcept {
    return _cameraIndex;
}

int VideoOperator::getFrameWidth() const noexcept {
    return _frameWidth;
}

int VideoOperator::getFrameHeight() const noexcept {
    return _frameHeight;
}

std::shared_ptr<cv::VideoCapture> VideoOperator::getCapture() const {
    std::lock_guard<std::mutex> lock(_capMutex);
    return _cap;
}

