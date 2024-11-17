
#ifndef MEASUREMENTSWINDOW_H
#define MEASUREMENTSWINDOW_H

#include "final/final.h"
#include "sub_window.h"
#include <deque>


using namespace finalcut;

class MeasurementsWindow : public SubWindow
{
  public:
    explicit MeasurementsWindow (finalcut::FWidget* = nullptr);

    MeasurementsWindow (const MeasurementsWindow&) = delete;

    MeasurementsWindow (MeasurementsWindow&&) noexcept = delete;

    ~MeasurementsWindow() noexcept override;

    auto operator = (const MeasurementsWindow&) -> MeasurementsWindow& = delete;

    auto operator = (MeasurementsWindow&&) noexcept -> MeasurementsWindow& = delete;

private:

    finalcut::FLineEdit _compassAngle {this};
    finalcut::FLineEdit _distance {this};
    finalcut::FLineEdit _current {this};
    finalcut::FLineEdit _voltage {this};



};

#endif // GRAPHWINDOW_H