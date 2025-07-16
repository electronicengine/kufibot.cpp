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
    int magnitude = _magnitude.getValue();
    //_robotControllerService->control_body(90, magnitude);
}

void BodyControllerWindow::backward()
{
    int magnitude = _magnitude.getValue();
    //_robotControllerService->control_body(-90, magnitude);
}

void BodyControllerWindow::stop()
{
    //_robotControllerService->control_body(0, 0);
}

void BodyControllerWindow::turnRight()
{
    int magnitude = _magnitude.getValue();
    //_robotControllerService->control_body(0, magnitude);
}

void BodyControllerWindow::turnLeft()
{
    int magnitude = _magnitude.getValue();
    //_robotControllerService->control_body(180, magnitude);
}
