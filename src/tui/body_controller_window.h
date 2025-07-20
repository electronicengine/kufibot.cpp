
#ifndef MOTORCONTROLLERWINDOW_H
#define MOTORCONTROLLERWINDOW_H

#include "final/final.h"
#include "sub_window.h"
#include <deque>

#undef K
#undef null

#include "../public_data_messages.h"


using namespace finalcut;

class BodyControllerWindow : public SubWindow
{
  public:
    explicit BodyControllerWindow (finalcut::FWidget* parent = nullptr);

    BodyControllerWindow (const BodyControllerWindow&) = delete;

    BodyControllerWindow (BodyControllerWindow&&) noexcept = delete;

    ~BodyControllerWindow() noexcept override;

    auto operator = (const BodyControllerWindow&) -> BodyControllerWindow& = delete;

    auto operator = (BodyControllerWindow&&) noexcept -> BodyControllerWindow& = delete;

    void setControlFunctionCallBack(std::function<void(const ControlData&)> controlFunctionCallBack) {
        _controlRobotFunctionCallBack = controlFunctionCallBack;
    }

    std::function<void(const std::map<ServoMotorJoint, uint8_t>&)> get_servo_joints_callback_function() {
        return std::bind(&BodyControllerWindow::update_servo_joints_callback, this, std::placeholders::_1);
    }

private:
    std::function<void(const ControlData&)> _controlRobotFunctionCallBack;
    FLabel _dcMotorControlLabel{"Dc Motor Controller", this};
    FButton _bodyForwardButton{finalcut::UniChar::BlackUpPointingTriangle, this};
    FButton _bodyTurnLeftButton{finalcut::UniChar::BlackLeftPointingTriangle, this};
    FButton _bodyTurnRightButton{finalcut::UniChar::BlackRightPointingTriangle, this};
    FButton _bodyBackwardButton{finalcut::UniChar::BlackDownPointingTriangle, this};
    FButton _bodyStopButton{finalcut::UniChar::BlackCircle, this};
    FSpinBox _dcMotorSpeed{this};

    FLabel _servoMotorControlLabel{"Servo Motor Controller", this};
    FButton _headUpButton{finalcut::UniChar::BlackUpPointingTriangle, this};
    FButton _headLeftButton{finalcut::UniChar::BlackLeftPointingTriangle, this};
    FButton _headRightButton{finalcut::UniChar::BlackRightPointingTriangle, this};
    FButton _headDownButton{finalcut::UniChar::BlackDownPointingTriangle, this};
    FButton _headDefaultPosButton{finalcut::UniChar::BlackCircle, this};
    FSpinBox _servoMotorSpeed{this};

    FLabel _leftArmlabel {this};
    FLineEdit _leftArmAngle {this};

    FLabel _rightArmlabel {this};
    FLineEdit _rigthArmAngle {this};

    FLabel _neckLabel {this};
    FLineEdit _neckAngle {this};

    FLabel _headUpDownLabel {this};
    FLineEdit _headUpDownAngle {this};

    FLabel _headLeftRightLabel {this};
    FLineEdit _headLeftRightAngle {this};

    FLabel _eyeLeftLabel {this};
    FLineEdit _eyeLeftAngle {this};

    FLabel _eyeRightLabel {this};
    FLineEdit _eyeRightAngle {this};

    void forward();
    void backward();
    void stop();
    void turnRight();
    void turnLeft();

    void headUp();
    void headDown();
    void headLeft();
    void headRight();
    void headDefaultPos();

    void update_servo_joints_callback(const std::map<ServoMotorJoint, uint8_t>& jointAngles);

};

#endif // GRAPHWINDOW_H