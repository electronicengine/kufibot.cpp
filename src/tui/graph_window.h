
#ifndef GRAPHWINDOW_H
#define GRAPHWINDOW_H

#include "final/final.h"
#include "sub_window.h"
#include <utility>
#include <deque>
#undef K
#undef null

#include "../subscriber.h"

using namespace finalcut;

class GraphWindow : public SubWindow
{
  public:
    explicit GraphWindow (finalcut::FWidget* = nullptr, double xMin = -10, double xMax = 10, double yMin = -10, double yMax = 10);

    GraphWindow (const GraphWindow&) = delete;

    GraphWindow (GraphWindow&&) noexcept = delete;

    ~GraphWindow() noexcept override;

    auto operator = (const GraphWindow&) -> GraphWindow& = delete;

    auto operator = (GraphWindow&&) noexcept -> GraphWindow& = delete;

    void add_point(double x, double y); 
    void add_realtime_point(double val, double interval = 1);
    void draw_line(double x1, double y1, double x2, double y2, FColor color);
    void draw_line(double angle, double magnitude, FColor color, bool overWritePrevious = true);
    void update_sensor_values(Json values);

  private:
    void draw() override;

    std::deque<std::pair<double, double>> _points;
    std::deque<FPoint> _lastLinePoints;
    std::size_t _maxPoints{40};                  
    double _xMin, _xMax, _yMin, _yMax; 

    FPoint transform_to_screen(double x, double y) const;

    void onClose (finalcut::FCloseEvent*) override;
    void onShow  (finalcut::FShowEvent*) override;
};

#endif // GRAPHWINDOW_H