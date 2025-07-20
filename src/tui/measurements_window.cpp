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

}

MeasurementsWindow::~MeasurementsWindow() noexcept
{

}

void MeasurementsWindow::onClose(finalcut::FCloseEvent *)
{
    hide();
    auto  parent = getParent();
    activate_window((FDialog*) parent);
}

void MeasurementsWindow::onShow(finalcut::FShowEvent *)
{
    activate_window(this);

}

void MeasurementsWindow::update_sensor_data_callback(const SensorData &data) {
    std::ostringstream stream;

    _compassAngle.setText(std::to_string(data.compassData->angle) + " °");
    _compassAngle.redraw();

    _distance.setText(std::to_string(data.distanceData->distance) + " cm");
    _distance.redraw();

    stream << std::fixed << std::setprecision(2) << data.powerData->current;
    _current.setText(stream.str() + " mA");    _current.redraw();

    _voltage.setText(std::to_string(data.powerData->busVoltage) + "  V");
    _voltage.redraw();

}
