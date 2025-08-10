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

#include "graph_window.h"
// Constructor
GraphWindow::GraphWindow(finalcut::FWidget* parent, double xMin, double xMax, double yMin, double yMax)
  : SubWindow(parent), _xMin(xMin), _xMax(xMax), _yMin(yMin), _yMax(yMax)
{
}

GraphWindow::~GraphWindow() noexcept = default;

void GraphWindow::add_point(double x, double y)
{
  if (_points.size() >= _maxPoints)
    _points.pop_front(); 

  _points.emplace_back(x, y);
  redraw(); 
}

void GraphWindow::add_realtime_point(double val, double interval)
{
    static double x = _xMin;
    add_point(x, val);
    x += interval;

    if (x > _xMax)
      x = _xMin;

    redraw();
    flush();
}

FPoint GraphWindow::transform_to_screen(double x, double y) const
{
  int width = getWidth();
  int height = getHeight();
  
  int screenX = static_cast<int>(((x - _xMin) / (_xMax - _xMin)) * (width - 1));
  int screenY = static_cast<int>(height - 1 - ((y - _yMin) / (_yMax - _yMin)) * (height - 1));
  
  return FPoint{screenX, screenY};
}

void GraphWindow::onClose(finalcut::FCloseEvent *close_event) {
  // RobotControllerService *robotControllerService = RobotControllerService::get_instance();
  // robotControllerService->un_subscribe(this);
  hide();
  auto  parent = getParent();
  activate_window((FDialog*) parent);
}

void GraphWindow::onShow(finalcut::FShowEvent *show_event) {
  // RobotControllerService *robotControllerService = RobotControllerService::get_instance();
  // robotControllerService->subscribe(this);
  activate_window(this);
}


void GraphWindow::draw()
{
  finalcut::FDialog::draw();

  setColor(FColor::LightGreen, FColor::Black);
  int width = getWidth();
  int height = getHeight();

  int zeroY = transform_to_screen(0, 0).getY();
  for (int x = 0; x < width; ++x)
    print() << FPoint{x, zeroY} << '-';

  int zeroX = transform_to_screen(0, 0).getX();
  for (int y = 0; y < height; ++y)
    print() << FPoint{zeroX, y} << '|';

  setColor(FColor::Red, FColor::Black);

  int xStep = static_cast<int>((_xMax - _xMin) / 10); 
  for (double x = _xMin; x <= _xMax; x += xStep)
  {
    FPoint screenPoint = transform_to_screen(x, 0);
    print() << screenPoint << ' ' << static_cast<int>(x); 
  }

  int yStep = static_cast<int>((_yMax - _yMin) / 10); 
  for (double y = _yMin; y <= _yMax; y += yStep)
  {
    FPoint screenPoint = transform_to_screen(0, y);
    print() << screenPoint << ' ' << static_cast<int>(y);
  }
  setColor(FColor::White, FColor::Black);

  for (const auto& [x, y] : _points)
  {
    FPoint screenPoint = transform_to_screen(x, y);
    print() << screenPoint << UniChar::BlackCircle;
  }

  setColor(FColor::LightGreen, FColor::Black);

}

void GraphWindow::draw_line(double x1, double y1, double x2, double y2, FColor color)
{
  setColor(color, FColor::Black);

  FPoint start = transform_to_screen(x1, y1);
  FPoint end = transform_to_screen(x2, y2);

  int dx = std::abs(end.getX() - start.getX());
  int dy = std::abs(end.getY() - start.getY());
  int sx = (start.getX() < end.getX()) ? 1 : -1;
  int sy = (start.getY() < end.getY()) ? 1 : -1;
  int err = dx - dy;

  int x = start.getX();
  int y = start.getY();

  while (true)
  {
    print() << FPoint{x, y} << UniChar::BlackCircle; 
    if (x == end.getX() && y == end.getY())
      break;

    int e2 = 2 * err;
    if (e2 > -dy)
    {
      err -= dy;
      x += sx;
    }
    if (e2 < dx)
    {
      err += dx;
      y += sy;
    }
  }
}

void GraphWindow::draw_line(double angle, double magnitude, FColor color, bool overWritePrevious)
{
  // 1. Eğer overwrite isteniyorsa, önceki çizgiyi sil
  if (overWritePrevious)
  {
    setColor(FColor::Black, FColor::Black); // Arka plan rengi ile sil
    for (const auto& p : _lastLinePoints)
      print() << p << ' ';  // Boş karakterle üstünü kapat
    _lastLinePoints.clear();
  }

  // 2. Yeni çizgi için renk ayarla
  setColor(color, FColor::Black);

  const double radians = angle * (M_PI / 180.0);
  double xEnd = magnitude * std::cos(radians);
  double yEnd = magnitude * std::sin(radians);

  FPoint start = transform_to_screen(0, 0);
  FPoint end = transform_to_screen(xEnd, yEnd);

  int x0 = start.getX();
  int y0 = start.getY();
  int x1 = end.getX();
  int y1 = end.getY();

  // Dikey çizgi özel durum
  if (x0 == x1) {
    std::deque<FPoint> newPoints;
    int yStart = std::min(y0, y1);
    int yEnd = std::max(y0, y1);
    for (int y = yStart; y <= yEnd; ++y)
    {
      FPoint p{x0, y};
      print() << p << UniChar::BlackCircle;
      newPoints.push_back(p);
    }
    _lastLinePoints = std::move(newPoints);
    return;
  }

  // Eğimi hesapla (float olarak)
  double slope = static_cast<double>(y1 - y0) / static_cast<double>(x1 - x0);
  double intercept = y0 - slope * x0;

  // 3. Her satır için sadece bir tane nokta çiz
  std::deque<FPoint> newPoints;
  int yStart = std::min(y0, y1);
  int yLast = std::max(y0, y1);
  for (int y = yStart; y <= yLast; ++y)
  {
    // y = slope * x + intercept => x = (y - intercept) / slope
    int x = static_cast<int>((y - intercept) / slope);
    if (x >= 0 && x < (int)getWidth())
    {
      FPoint p{x, y};
      print() << p << UniChar::BlackCircle;
      newPoints.push_back(p);
    }
  }

  _lastLinePoints = std::move(newPoints); // Yeni çizilen çizgiyi kaydet
}




