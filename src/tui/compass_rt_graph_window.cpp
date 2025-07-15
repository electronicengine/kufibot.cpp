#include "compass_rt_graph_window.h"

CompassRtGraphWindow::CompassRtGraphWindow(finalcut::FWidget *parent, double xMin, double xMax, double yMin, double yMax):
    GraphWindow(parent, xMin, xMax, yMin, yMax)
{
    setText("Compass RT Graph");
}

CompassRtGraphWindow::~CompassRtGraphWindow() noexcept
{
}
