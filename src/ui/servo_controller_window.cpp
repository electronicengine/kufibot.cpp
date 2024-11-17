#include "servo_controller_window.h"

ServoControllerWindow::ServoControllerWindow(finalcut::FWidget *parent) : SubWindow(parent)
{
    setText("Servo Controller");

    _rightArm.setGeometry(finalcut::FPoint{12,3}, finalcut::FSize{7, 1});
    _rightArm.setLabelText ("Right Arm");
    _rightArm.setRange (0, 180);
    _rightArm.setValue (0);

    _leftArm.setGeometry(finalcut::FPoint{12,6}, finalcut::FSize{7, 1});
    _leftArm.setLabelText ("Left Arm");
    _leftArm.setRange (0, 180);
    _leftArm.setValue (0);

    _neckDown.setGeometry(finalcut::FPoint{12,9}, finalcut::FSize{7, 1});
    _neckDown.setLabelText ("Neck Down");
    _neckDown.setRange (0, 180);
    _neckDown.setValue (0);

    _neckUp.setGeometry(finalcut::FPoint{12, 12}, finalcut::FSize{7, 1});
    _neckUp.setLabelText ("Neck Up");
    _neckUp.setRange (0, 180);
    _neckUp.setValue (0);

    _neckRight.setGeometry(finalcut::FPoint{12,15}, finalcut::FSize{7, 1});
    _neckRight.setLabelText ("Neck Right");
    _neckRight.setRange (0, 180);
    _neckRight.setValue (0);

    _eyeLeft.setGeometry(finalcut::FPoint{12,18}, finalcut::FSize{7, 1});
    _eyeLeft.setLabelText ("Eye Left");
    _eyeLeft.setRange (0, 180);
    _eyeLeft.setValue (0);

    _eyeRight.setGeometry(finalcut::FPoint{12,21}, finalcut::FSize{7, 1});
    _eyeRight.setLabelText ("Eye Right");
    _eyeRight.setRange (0, 180);
    _eyeRight.setValue (0);

    _setValues.setGeometry(finalcut::FPoint{12,25}, finalcut::FSize{8, 2});
}

ServoControllerWindow::~ServoControllerWindow() noexcept
{

}
