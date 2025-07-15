
#ifndef COMPASSRTGRAPHWINDOW_H
#define COMPASSRTGRAPHWINDOW_H

#include "final/final.h"
#include "graph_window.h"
#include <deque>


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

 

};

#endif // GRAPHWINDOW_H