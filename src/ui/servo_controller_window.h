
#ifndef SERVOCONTROLLERWINDOW_H
#define SERVOCONTROLLERWINDOW_H

#include "final/final.h"
#include "sub_window.h"
#include <deque>


using namespace finalcut;

class ServoControllerWindow : public SubWindow
{
  public:
    explicit ServoControllerWindow (finalcut::FWidget* = nullptr);

    ServoControllerWindow (const ServoControllerWindow&) = delete;

    ServoControllerWindow (ServoControllerWindow&&) noexcept = delete;

    ~ServoControllerWindow() noexcept override;

    auto operator = (const ServoControllerWindow&) -> ServoControllerWindow& = delete;

    auto operator = (ServoControllerWindow&&) noexcept -> ServoControllerWindow& = delete;

private:
    FSpinBox _rightArm{this};
    FSpinBox _leftArm{this};
    FSpinBox _neckDown{this};
    FSpinBox _neckUp{this};
    FSpinBox _neckRight{this};
    FSpinBox _eyeLeft{this};
    FSpinBox _eyeRight{this};
    FButton _setValues{"Save", this};

};

#endif // GRAPHWINDOW_H