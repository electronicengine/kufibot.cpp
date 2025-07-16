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

    _neck.setGeometry(finalcut::FPoint{12,9}, finalcut::FSize{7, 1});
    _neck.setLabelText ("Neck Down");
    _neck.setRange (0, 180);
    _neck.setValue (0);

    _headUpDown.setGeometry(finalcut::FPoint{12, 12}, finalcut::FSize{7, 1});
    _headUpDown.setLabelText ("Head Up");
    _headUpDown.setRange (0, 180);
    _headUpDown.setValue (0);

    _headLeftRight.setGeometry(finalcut::FPoint{12,15}, finalcut::FSize{7, 1});
    _headUpDown.setLabelText ("Head Left");
    _headUpDown.setRange (0, 180);
    _headUpDown.setValue (0);

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

}

ServoControllerWindow::~ServoControllerWindow() noexcept
{

}

void ServoControllerWindow::set_servo_values()
{
    ControlData data;

    data.jointAngles.emplace();
    data.jointAngles.value()[ServoMotorJoint::rightArm] = _rightArm.getValue();
    data.jointAngles.value()[ServoMotorJoint::leftArm] = _leftArm.getValue();
    data.jointAngles.value()[ServoMotorJoint::neck] = _neck.getValue();
    data.jointAngles.value()[ServoMotorJoint::headUpDown] = _headUpDown.getValue();
    data.jointAngles.value()[ServoMotorJoint::headLeftRight] = _headLeftRight.getValue();
    data.jointAngles.value()[ServoMotorJoint::eyeLeft] = _eyeLeft.getValue();
    data.jointAngles.value()[ServoMotorJoint::eyeRight] = _eyeRight.getValue();

    _controlRobotFunctionCallBack(data);
}

void ServoControllerWindow::update_servo_joints_callback(const std::map<ServoMotorJoint, uint8_t> &jointAngles) {

    _rightArm.setValue (jointAngles.at(ServoMotorJoint::rightArm));
    _rightArm.redraw();

    _leftArm.setValue (jointAngles.at(ServoMotorJoint::leftArm));
    _leftArm.redraw();

    _neck.setValue (jointAngles.at(ServoMotorJoint::neck));
    _neck.redraw();

    _headUpDown.setValue (jointAngles.at(ServoMotorJoint::headUpDown));
    _headUpDown.redraw();

    _headUpDown.setValue (jointAngles.at(ServoMotorJoint::headUpDown));
    _headUpDown.redraw();

    _eyeLeft.setValue (jointAngles.at(ServoMotorJoint::eyeLeft));
    _eyeLeft.redraw();

    _eyeRight.setValue (jointAngles.at(ServoMotorJoint::eyeRight));
    _eyeRight.redraw();
}

void ServoControllerWindow::onShow(finalcut::FShowEvent *)
{
    activate_window(this);
}

