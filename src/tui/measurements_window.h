
#ifndef MEASUREMENTSWINDOW_H
#define MEASUREMENTSWINDOW_H


#include "final/final.h"
#include "sub_window.h"
#include <deque>

#undef K
#undef null

#include "../subscriber.h"
#include "../services/robot_controller_service.h"



class MeasurementsWindow : public SubWindow, public Subscriber
{
  public:
    explicit MeasurementsWindow (finalcut::FWidget* = nullptr);

    MeasurementsWindow (const MeasurementsWindow&) = delete;

    MeasurementsWindow (MeasurementsWindow&&) noexcept = delete;

    ~MeasurementsWindow() noexcept override;

    auto operator = (const MeasurementsWindow&) -> MeasurementsWindow& = delete;

    auto operator = (MeasurementsWindow&&) noexcept -> MeasurementsWindow& = delete;
    void update_sensor_values(Json values) override;


private:

    finalcut::FLineEdit _compassAngle {this};
    finalcut::FLineEdit _distance {this};
    finalcut::FLineEdit _current {this};
    finalcut::FLineEdit _voltage {this};

    RobotControllerService *_robotControllerService;

    void onClose (finalcut::FCloseEvent*) override;
    void onShow  (finalcut::FShowEvent*) override;

};

#endif // GRAPHWINDOW_H