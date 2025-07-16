
#ifndef MEASUREMENTSWINDOW_H
#define MEASUREMENTSWINDOW_H


#include "final/final.h"
#include "sub_window.h"
#include <deque>

#undef K
#undef null

#include "../public_data_messages.h"


class MeasurementsWindow : public SubWindow
{
  public:
    explicit MeasurementsWindow (finalcut::FWidget* = nullptr);

    MeasurementsWindow (const MeasurementsWindow&) = delete;

    MeasurementsWindow (MeasurementsWindow&&) noexcept = delete;

    ~MeasurementsWindow() noexcept override;

    auto operator = (const MeasurementsWindow&) -> MeasurementsWindow& = delete;

    auto operator = (MeasurementsWindow&&) noexcept -> MeasurementsWindow& = delete;

    std::function<void(const SensorData&)> get_sensor_data_callback_function() {
        return std::bind(&MeasurementsWindow::update_sensor_data_callback, this, std::placeholders::_1);
    }

private:
    finalcut::FLineEdit _compassAngle {this};
    finalcut::FLineEdit _distance {this};
    finalcut::FLineEdit _current {this};
    finalcut::FLineEdit _voltage {this};

    void onClose (finalcut::FCloseEvent*) override;
    void onShow  (finalcut::FShowEvent*) override;
    void update_sensor_data_callback(const SensorData& data);

};

#endif // GRAPHWINDOW_H