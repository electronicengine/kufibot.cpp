
#ifndef SERVOCONTROLLERWINDOW_H
#define SERVOCONTROLLERWINDOW_H

#include "final/final.h"
#include "sub_window.h"
#include <deque>


#undef K
#undef null

#include "../services/robot_controller_service.h"


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
    FButton _setValuesButton{"Save", this};

    RobotControllerService *_robotControllerService;

    void set_servo_values();
    void show_current_servo_values();

protected:

void onShow(finalcut::FShowEvent *) override;

};

#endif // GRAPHWINDOW_H