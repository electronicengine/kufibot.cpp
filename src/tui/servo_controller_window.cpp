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

    _setValuesButton.setGeometry(finalcut::FPoint{12,25}, finalcut::FSize{8, 2});
    add_clicked_callback (&_setValuesButton, this, &ServoControllerWindow::set_servo_values);
    _robotControllerService = RobotControllerService::get_instance();

}

ServoControllerWindow::~ServoControllerWindow() noexcept
{

}

void ServoControllerWindow::set_servo_values()
{
    std::map<std::string, int> values;

    values["rigth_arm"] = _rightArm.getValue();
    values["left_arm"] = _leftArm.getValue();
    values["neck_down"] = _neckDown.getValue();
    values["neck_up"] = _neckUp.getValue();
    values["neck_right"] = _neckRight.getValue();
    values["eye_left"] = _eyeLeft.getValue();
    values["eye_right"] = _eyeRight.getValue();

    _robotControllerService->set_servo_joint_map(values);

}

void ServoControllerWindow::show_current_servo_values()
{

    std::map<std::string, int> values = _robotControllerService->get_servo_joint_map();

    _rightArm.setValue (values["rigth_arm"]);
    _rightArm.redraw();

    _leftArm.setValue (values["left_arm"]);
    _leftArm.redraw();

    _neckDown.setValue (values["neck_down"]);
    _neckDown.redraw();

    _neckUp.setValue (values["neck_up"]);
    _neckUp.redraw();

    _neckRight.setValue (values["neck_right"]);
    _neckRight.redraw();

    _eyeLeft.setValue (values["eye_left"]);
    _eyeLeft.redraw();

    _eyeRight.setValue (values["eye_right"]);
    _eyeRight.redraw();

}

void ServoControllerWindow::onShow(finalcut::FShowEvent *)
{
    activate_window(this);
    show_current_servo_values();
}

