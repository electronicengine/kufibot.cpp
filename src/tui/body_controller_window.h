
#ifndef MOTORCONTROLLERWINDOW_H
#define MOTORCONTROLLERWINDOW_H

#include "final/final.h"
#include "sub_window.h"
#include <deque>

#undef K
#undef null

#include "../public_data_messages.h"

#undef K
#undef null

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


private:
    std::function<void(const ControlData&)> _controlRobotFunctionCallBack;

    FButton _bodyForwardButton{finalcut::UniChar::BlackUpPointingTriangle, this};
    FButton _bodyTurnLeftButton{finalcut::UniChar::BlackLeftPointingTriangle, this};
    FButton _bodyTurnRightButton{finalcut::UniChar::BlackRightPointingTriangle, this};
    FButton _bodyBackwardButton{finalcut::UniChar::BlackDownPointingTriangle, this};
    FButton _bodyStopButton{finalcut::UniChar::BlackCircle, this};
    FSpinBox _magnitude{this};

    void forward();
    void backward();
    void stop();
    void turnRight();
    void turnLeft();


};

#endif // GRAPHWINDOW_H