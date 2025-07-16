#include "body_controller_window.h"

BodyControllerWindow::BodyControllerWindow(finalcut::FWidget *parent) : SubWindow(parent)
{
    setText("Body Controller");
    _bodyForwardButton.setGeometry(finalcut::FPoint{10,1}, finalcut::FSize{7, 3});
    _bodyTurnLeftButton.setGeometry(finalcut::FPoint{1,6}, finalcut::FSize{7, 3});
    _bodyTurnRightButton.setGeometry(finalcut::FPoint{19,6}, finalcut::FSize{7, 3});
    _bodyBackwardButton.setGeometry(finalcut::FPoint{10,11}, finalcut::FSize{7, 3});
    _bodyStopButton.setGeometry(finalcut::FPoint{10,6}, finalcut::FSize{7, 3});

    _magnitude.setGeometry(finalcut::FPoint{10,16}, finalcut::FSize{9, 1});
    _magnitude.setLabelText ("Magnitude ");
    _magnitude.setRange (0, 100);
    _magnitude.setValue (100);

    add_clicked_callback(&_bodyForwardButton, this, &BodyControllerWindow::forward);
    add_clicked_callback(&_bodyBackwardButton, this, &BodyControllerWindow::backward);
    add_clicked_callback(&_bodyTurnLeftButton, this, &BodyControllerWindow::turnLeft);
    add_clicked_callback(&_bodyTurnRightButton, this, &BodyControllerWindow::turnRight);
    add_clicked_callback(&_bodyStopButton, this, &BodyControllerWindow::stop);
}

BodyControllerWindow::~BodyControllerWindow() noexcept
{

}

void BodyControllerWindow::forward()
{
    ControlData data;

    data.bodyJoystick.emplace();
    data.bodyJoystick->angle = UP;
    data.bodyJoystick->strength = _magnitude.getValue();
    _controlRobotFunctionCallBack(data);

}

void BodyControllerWindow::backward()
{
    ControlData data;

    data.bodyJoystick.emplace();
    data.bodyJoystick->angle = DOWN;
    data.bodyJoystick->strength = _magnitude.getValue();
    _controlRobotFunctionCallBack(data);
}

void BodyControllerWindow::stop()
{
    ControlData data;

    data.bodyJoystick.emplace();
    data.bodyJoystick->angle = 0;
    data.bodyJoystick->strength = 0;
    _controlRobotFunctionCallBack(data);}

void BodyControllerWindow::turnRight()
{
    ControlData data;

    data.bodyJoystick.emplace();
    data.bodyJoystick->angle = RIGHT;
    data.bodyJoystick->strength = _magnitude.getValue();
    _controlRobotFunctionCallBack(data);
}

void BodyControllerWindow::turnLeft()
{
    ControlData data;

    data.bodyJoystick.emplace();
    data.bodyJoystick->angle = LEFT;
    data.bodyJoystick->strength = _magnitude.getValue();
    _controlRobotFunctionCallBack(data);
}
