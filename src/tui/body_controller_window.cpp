#include "body_controller_window.h"

BodyControllerWindow::BodyControllerWindow(finalcut::FWidget *parent) : SubWindow(parent)
{
    setText("Body Controller");
    _dcMotorControlLabel.setGeometry(finalcut::FPoint{5,1}, finalcut::FSize{24, 1});

    _bodyForwardButton.setGeometry(finalcut::FPoint{11,2}, finalcut::FSize{7, 3});
    _bodyTurnLeftButton.setGeometry(finalcut::FPoint{2,7}, finalcut::FSize{7, 3});
    _bodyTurnRightButton.setGeometry(finalcut::FPoint{20,7}, finalcut::FSize{7, 3});
    _bodyBackwardButton.setGeometry(finalcut::FPoint{11,12}, finalcut::FSize{7, 3});
    _bodyStopButton.setGeometry(finalcut::FPoint{11,7}, finalcut::FSize{7, 3});

    _dcMotorSpeed.setGeometry(finalcut::FPoint{11,17}, finalcut::FSize{9, 1});
    _dcMotorSpeed.setLabelText ("Speed ");
    _dcMotorSpeed.setRange (0, 100);
    _dcMotorSpeed.setValue (100);

    _servoMotorControlLabel.setGeometry(finalcut::FPoint{32,1}, finalcut::FSize{24, 1});

    _headUpButton.setGeometry(finalcut::FPoint{40,2}, finalcut::FSize{7, 3});
    _headLeftButton.setGeometry(finalcut::FPoint{31,7}, finalcut::FSize{7, 3});
    _headRightButton.setGeometry(finalcut::FPoint{50,7}, finalcut::FSize{7, 3});
    _headDownButton.setGeometry(finalcut::FPoint{40,12}, finalcut::FSize{7, 3});
    _headDefaultPosButton.setGeometry(finalcut::FPoint{40,7}, finalcut::FSize{7, 3});

    _servoMotorSpeed.setGeometry(finalcut::FPoint{40,17}, finalcut::FSize{9, 1});
    _servoMotorSpeed.setLabelText ("Speed");
    _servoMotorSpeed.setRange (0, 100);
    _servoMotorSpeed.setValue (100);

    _leftArmlabel.setText("Left Arm");
    _leftArmlabel.setGeometry(finalcut::FPoint{62,1}, finalcut::FSize{27, 1});
    _leftArmAngle.setGeometry(finalcut::FPoint{62,2}, finalcut::FSize{7, 1});

    _rightArmlabel.setText("Right Arm");
    _rightArmlabel.setGeometry(finalcut::FPoint{62,4}, finalcut::FSize{27, 1});
    _rigthArmAngle.setGeometry(finalcut::FPoint{62,5}, finalcut::FSize{7, 1});

    _neckLabel.setText("Neck");
    _neckLabel.setGeometry(finalcut::FPoint{62,7}, finalcut::FSize{27, 1});
    _neckAngle.setGeometry(finalcut::FPoint{62,8}, finalcut::FSize{7, 1});

    _headUpDownLabel.setText("Head Up");
    _headUpDownLabel.setGeometry(finalcut::FPoint{62,10}, finalcut::FSize{27, 1});
    _headUpDownAngle.setGeometry(finalcut::FPoint{62,11}, finalcut::FSize{7, 1});

    _headLeftRightLabel.setText("Head Left");
    _headLeftRightLabel.setGeometry(finalcut::FPoint{62,13}, finalcut::FSize{27, 1});
    _headLeftRightAngle.setGeometry(finalcut::FPoint{62,14}, finalcut::FSize{7, 1});

    _eyeLeftLabel.setText("Eye Left");
    _eyeLeftLabel.setGeometry(finalcut::FPoint{62,16}, finalcut::FSize{27, 1});
    _eyeLeftAngle.setGeometry(finalcut::FPoint{62,17}, finalcut::FSize{7, 1});

    _eyeRightLabel.setText("Eye Right");
    _eyeRightLabel.setGeometry(finalcut::FPoint{62,14}, finalcut::FSize{27, 1});
    _eyeRightAngle.setGeometry(finalcut::FPoint{62,14}, finalcut::FSize{7, 1});

    add_clicked_callback(&_bodyForwardButton, this, &BodyControllerWindow::forward);
    add_clicked_callback(&_bodyBackwardButton, this, &BodyControllerWindow::backward);
    add_clicked_callback(&_bodyTurnLeftButton, this, &BodyControllerWindow::turnLeft);
    add_clicked_callback(&_bodyTurnRightButton, this, &BodyControllerWindow::turnRight);
    add_clicked_callback(&_bodyStopButton, this, &BodyControllerWindow::stop);


    add_clicked_callback(&_headUpButton, this, &BodyControllerWindow::headUp);
    add_clicked_callback(&_headDownButton, this, &BodyControllerWindow::headDown);
    add_clicked_callback(&_headLeftButton, this, &BodyControllerWindow::headLeft);
    add_clicked_callback(&_headRightButton, this, &BodyControllerWindow::headRight);
    add_clicked_callback(&_headDefaultPosButton, this, &BodyControllerWindow::headDefaultPos);
}

BodyControllerWindow::~BodyControllerWindow() noexcept
{

}

void BodyControllerWindow::forward()
{
    ControlData data;

    data.bodyJoystick.emplace();
    data.bodyJoystick->angle = UP;
    data.bodyJoystick->strength = _dcMotorSpeed.getValue();
    _controlRobotFunctionCallBack(data);

}

void BodyControllerWindow::backward()
{
    ControlData data;

    data.bodyJoystick.emplace();
    data.bodyJoystick->angle = DOWN;
    data.bodyJoystick->strength = _dcMotorSpeed.getValue();
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
    data.bodyJoystick->strength = _dcMotorSpeed.getValue();
    _controlRobotFunctionCallBack(data);
}

void BodyControllerWindow::turnLeft()
{
    ControlData data;

    data.bodyJoystick.emplace();
    data.bodyJoystick->angle = LEFT;
    data.bodyJoystick->strength = _dcMotorSpeed.getValue();
    _controlRobotFunctionCallBack(data);
}

void BodyControllerWindow::headUp() {
    ControlData data;

    data.headJoystick.emplace();
    data.headJoystick->angle = UP;
    data.headJoystick->strength = _dcMotorSpeed.getValue();
    _controlRobotFunctionCallBack(data);
}

void BodyControllerWindow::headDown() {
    ControlData data;

    data.headJoystick.emplace();
    data.headJoystick->angle = DOWN;
    data.headJoystick->strength = _dcMotorSpeed.getValue();
    _controlRobotFunctionCallBack(data);
}

void BodyControllerWindow::headLeft() {
    ControlData data;

    data.headJoystick.emplace();
    data.headJoystick->angle = LEFT;
    data.headJoystick->strength = _dcMotorSpeed.getValue();
    _controlRobotFunctionCallBack(data);
}

void BodyControllerWindow::headRight() {
    ControlData data;

    data.headJoystick.emplace();
    data.headJoystick->angle = RIGHT;
    data.headJoystick->strength = _dcMotorSpeed.getValue();
    _controlRobotFunctionCallBack(data);
}

void BodyControllerWindow::headDefaultPos() {
    ControlData data;

    data.headJoystick.emplace();
    data.headJoystick->angle = 0;
    data.headJoystick->strength = _dcMotorSpeed.getValue();
    _controlRobotFunctionCallBack(data);
}

void BodyControllerWindow::update_servo_joints_callback(const std::map<ServoMotorJoint, uint8_t> &jointAngles) {

    _rigthArmAngle.setText (std::to_string(jointAngles.at(ServoMotorJoint::rightArm)));
    _rigthArmAngle.redraw();

    _leftArmAngle.setText (std::to_string(jointAngles.at(ServoMotorJoint::leftArm)));
    _leftArmAngle.redraw();

    _neckAngle.setText (std::to_string(jointAngles.at(ServoMotorJoint::neck)));
    _neckAngle.redraw();

    _headUpDownAngle.setText (std::to_string(jointAngles.at(ServoMotorJoint::headUpDown)));
    _headUpDownAngle.redraw();

    _headLeftRightAngle.setText (std::to_string(jointAngles.at(ServoMotorJoint::headLeftRight)));
    _headLeftRightAngle.redraw();

    _eyeLeftAngle.setText (std::to_string(jointAngles.at(ServoMotorJoint::eyeLeft)));
    _eyeLeftAngle.redraw();

    _eyeRightAngle.setText (std::to_string(jointAngles.at(ServoMotorJoint::eyeRight)));
    _eyeRightAngle.redraw();

}
