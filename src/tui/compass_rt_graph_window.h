
#ifndef COMPASSRTGRAPHWINDOW_H
#define COMPASSRTGRAPHWINDOW_H

#include "final/final.h"
#include "graph_window.h"
#include <deque>

#undef K
#undef null

#include "../public_data_messages.h"

using namespace finalcut;

class CompassRtGraphWindow : public GraphWindow
{
  public:
    explicit CompassRtGraphWindow (finalcut::FWidget* = nullptr, double xMin = -10, double xMax = 10, double yMin = -10, double yMax = 10);

    CompassRtGraphWindow (const CompassRtGraphWindow&) = delete;

    CompassRtGraphWindow (CompassRtGraphWindow&&) noexcept = delete;

    ~CompassRtGraphWindow() noexcept override;

    auto operator = (const CompassRtGraphWindow&) -> GraphWindow& = delete;

    auto operator = (CompassRtGraphWindow&&) noexcept -> CompassRtGraphWindow& = delete;

    std::function<void(const int&)> get_compas_direction_callback_function() {
        return std::bind(&CompassRtGraphWindow::update_compas_direction_callback, this, std::placeholders::_1);
    }


private:

    void draw_compas_direction(int angle);

    void update_compas_direction_callback(const int& angle);

};

#endif // GRAPHWINDOW_H