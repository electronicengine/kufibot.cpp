/*
* This file is part of Kufibot.
 *
 * Kufibot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kufibot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kufibot. If not, see <https://www.gnu.org/licenses/>.
 */

#include "compass_rt_graph_window.h"

CompassRtGraphWindow::CompassRtGraphWindow(finalcut::FWidget *parent, double xMin, double xMax, double yMin, double yMax):
    GraphWindow(parent, xMin, xMax, yMin, yMax)
{
    setText("Compass RT Graph");
}

CompassRtGraphWindow::~CompassRtGraphWindow() noexcept
{
}



void CompassRtGraphWindow::update_compas_direction_callback(const int &angle) {
    draw_line(angle, 80, FColor::Blue, true);
}
