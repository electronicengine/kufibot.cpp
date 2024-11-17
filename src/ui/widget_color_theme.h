#ifndef WIDGETCOLORTHEME_H
#define WIDGETCOLORTHEME_H

#include <final/final.h>

using namespace finalcut;

class AWidgetColorTheme final : public finalcut::FWidgetColors
{
  public:
    AWidgetColorTheme();
    ~AWidgetColorTheme() override;

    auto getClassName() const -> finalcut::FString override;
    void setColorTheme() override;
};

#endif  // WIDGETCOLORTHEME_H