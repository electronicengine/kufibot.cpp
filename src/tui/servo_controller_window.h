
#ifndef SERVOCONTROLLERWINDOW_H
#define SERVOCONTROLLERWINDOW_H

#include "final/final.h"
#include "sub_window.h"
#include <deque>

#undef K
#undef null

#include "../public_data_messages.h"



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

    void setControlFunctionCallBack(std::function<void(const ControlData&)> controlFunctionCallBack) {
        _controlRobotFunctionCallBack = controlFunctionCallBack;
    }

    std::function<void(const std::map<ServoMotorJoint, uint8_t>&)> get_servo_joints_callback_function() {
        return std::bind(&ServoControllerWindow::update_servo_joints_callback, this, std::placeholders::_1);
    }

private:
    std::function<void(const ControlData&)> _controlRobotFunctionCallBack;

    FSpinBox _rightArm{this};
    FSpinBox _leftArm{this};
    FSpinBox _neck{this};
    FSpinBox _headUpDown{this};
    FSpinBox _headLeftRight{this};
    FSpinBox _eyeLeft{this};
    FSpinBox _eyeRight{this};
    FButton _setValuesButton{"Save", this};

    void set_servo_values();

    void update_servo_joints_callback(const std::map<ServoMotorJoint, uint8_t>& jointAngles);

protected:

    void onShow(finalcut::FShowEvent *) override;

};

#endif // SERVOCONTROLLERWINDOW_H