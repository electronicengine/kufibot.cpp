#include "body_controller_window.h"

BodyControllerWindow::BodyControllerWindow(finalcut::FWidget *parent) : SubWindow(parent)
{
    setText("Body Controller");
    _bodyForwardButton.setGeometry(finalcut::FPoint{10,1}, finalcut::FSize{7, 3});
    _bodyTurnLeftButton.setGeometry(finalcut::FPoint{1,6}, finalcut::FSize{7, 3});
    _bodyTurnRightButton.setGeometry(finalcut::FPoint{19,6}, finalcut::FSize{7, 3});
    _bodyBackwardButton.setGeometry(finalcut::FPoint{10,11}, finalcut::FSize{7, 3});
    _bodyStopButton.setGeometry(finalcut::FPoint{10,6}, finalcut::FSize{7, 3});
}

BodyControllerWindow::~BodyControllerWindow() noexcept
{

}
