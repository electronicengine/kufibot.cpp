#include "measurements_window.h"
#include "main_window.h"

MeasurementsWindow::MeasurementsWindow(finalcut::FWidget *parent) : SubWindow(parent)
{
    setText("Measurements Window");

    _compassAngle.setGeometry(finalcut::FPoint{12,3}, finalcut::FSize{14, 1});
    _compassAngle.setLabelText ("Angle");
    _compassAngle.setDisable();

    _distance.setGeometry(finalcut::FPoint{12,6}, finalcut::FSize{14, 1});
    _distance.setLabelText ("Distance");
    _distance.setDisable();

    _current.setGeometry(finalcut::FPoint{12,9}, finalcut::FSize{14, 1});
    _current.setLabelText ("Current");
    _current.setDisable();

    _voltage.setGeometry(finalcut::FPoint{12,12}, finalcut::FSize{14, 1});
    _voltage.setLabelText ("Voltage");
    _voltage.setDisable();

    _robotControllerService = RobotControllerService::get_instance();

}

MeasurementsWindow::~MeasurementsWindow() noexcept
{

}

void MeasurementsWindow::update_sensor_values(Json values)
{

    _compassAngle.setText(values["compass"]["angle"].dump() + " °");
    _compassAngle.redraw();

    _distance.setText(values["distance"]["Distance"].dump() + " cm");
    _distance.redraw();

    _current.setText(values["power"]["BusCurrent"].dump().substr(0,6) + "  maH");
    _current.redraw();

    _voltage.setText(values["power"]["BusVoltage"].dump().substr(0,4) + "  V");
    _voltage.redraw();

}

void MeasurementsWindow::onClose(finalcut::FCloseEvent *)
{
    _robotControllerService->un_subscribe(this);
    hide();
    auto  parent = getParent();
    activate_window((FDialog*) parent);
}

void MeasurementsWindow::onShow(finalcut::FShowEvent *)
{
    _robotControllerService->subscribe(this);
    activate_window(this);

}
