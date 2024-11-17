#include "measurements_window.h"

MeasurementsWindow::MeasurementsWindow(finalcut::FWidget *parent) : SubWindow(parent)
{
    setText("Measurements Window");

    _compassAngle.setGeometry(finalcut::FPoint{12,3}, finalcut::FSize{7, 1});
    _compassAngle.setLabelText ("Angle");
    _compassAngle.setDisable();
    _compassAngle.setText("123 ");

    _distance.setGeometry(finalcut::FPoint{12,6}, finalcut::FSize{7, 1});
    _distance.setLabelText ("Distance");
    _distance.setDisable();
    _distance.setText("456");

    _current.setGeometry(finalcut::FPoint{12,9}, finalcut::FSize{7, 1});
    _current.setLabelText ("Current");
    _current.setDisable();
    _current.setText("454.54");

    _voltage.setGeometry(finalcut::FPoint{12,12}, finalcut::FSize{7, 1});
    _voltage.setLabelText ("Voltage");
    _voltage.setDisable();
    _voltage.setText("11.5");

}

MeasurementsWindow::~MeasurementsWindow() noexcept
{

}
