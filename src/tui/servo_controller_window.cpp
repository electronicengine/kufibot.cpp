#include "servo_controller_window.h"

ServoControllerWindow::ServoControllerWindow(finalcut::FWidget *parent) : SubWindow(parent)
{
    setText("Servo Controller");



    _rightArm.setGeometry(finalcut::FPoint{12,3}, finalcut::FSize{7, 1});
    _rightArm.setLabelText ("Right Arm");
    _rightArm.setRange (0, 180);

    _leftArm.setGeometry(finalcut::FPoint{12,6}, finalcut::FSize{7, 1});
    _leftArm.setLabelText ("Left Arm");
    _leftArm.setRange (0, 180);

    _neck.setGeometry(finalcut::FPoint{12,9}, finalcut::FSize{7, 1});
    _neck.setLabelText ("Neck");
    _neck.setRange (0, 180);

    _headUpDown.setGeometry(finalcut::FPoint{12, 12}, finalcut::FSize{7, 1});
    _headUpDown.setLabelText ("Head UpDown");
    _headUpDown.setRange (0, 180);

    _headLeftRight.setGeometry(finalcut::FPoint{12,15}, finalcut::FSize{7, 1});
    _headLeftRight.setLabelText ("Head LeftRight");
    _headLeftRight.setRange (0, 180);

    _eyeLeft.setGeometry(finalcut::FPoint{12,18}, finalcut::FSize{7, 1});
    _eyeLeft.setLabelText ("Eye Left");
    _eyeLeft.setRange (0, 180);
    _eyeLeft.setValue (0);

    _eyeRight.setGeometry(finalcut::FPoint{12,21}, finalcut::FSize{7, 1});
    _eyeRight.setLabelText ("Eye Right");
    _eyeRight.setRange (0, 180);

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


void ServoControllerWindow::onShow(finalcut::FShowEvent *)
{
    std::map<ServoMotorJoint, uint8_t> defaultJointAngles = {
        {ServoMotorJoint::rightArm, 15}, {ServoMotorJoint::leftArm, 170}, {ServoMotorJoint::neck, 78},
        {ServoMotorJoint::headUpDown, 15}, {ServoMotorJoint::headLeftRight, 90}, {ServoMotorJoint::eyeRight, 160},
        {ServoMotorJoint::eyeLeft, 20}
    };
    _rightArm.setValue(defaultJointAngles[ServoMotorJoint::rightArm]);
    _leftArm.setValue(defaultJointAngles[ServoMotorJoint::leftArm]);
    _neck.setValue(defaultJointAngles[ServoMotorJoint::neck]);
    _headUpDown.setValue(defaultJointAngles[ServoMotorJoint::headUpDown]);
    _headLeftRight.setValue(defaultJointAngles[ServoMotorJoint::headLeftRight]);
    _eyeLeft.setValue(defaultJointAngles[ServoMotorJoint::eyeLeft]);
    _eyeRight.setValue(defaultJointAngles[ServoMotorJoint::eyeRight]);

    activate_window(this);
}

