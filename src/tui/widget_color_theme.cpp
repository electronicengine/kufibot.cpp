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

#include "widget_color_theme.h"

AWidgetColorTheme::AWidgetColorTheme()
{
    setColorTheme();
}

AWidgetColorTheme::~AWidgetColorTheme() = default;

auto AWidgetColorTheme::getClassName() const -> finalcut::FString
{
    return "AWidgetColorTheme";
}

void AWidgetColorTheme::setColorTheme()
{
    term = {
        finalcut::FColor::LightBlue,      // Foreground
        finalcut::FColor::Black   // Background
    };

    dialog = {
        finalcut::FColor::LightGray,      // Foreground
        finalcut::FColor::Black,  // Background
        finalcut::FColor::Red,        // Resize foreground
        finalcut::FColor::Blue        // Emphasis foreground
    };

    text = {
        finalcut::FColor::LightGray,      // Foreground
        finalcut::FColor::Black,  // Background
        finalcut::FColor::White,      // Selected foreground
        finalcut::FColor::Blue,       // Selected background
        finalcut::FColor::White,      // Selected focused foreground
        finalcut::FColor::Cyan        // Selected focused background
    };

    error_box = {
        finalcut::FColor::Black,      // Foreground
        finalcut::FColor::Yellow,     // Background
        finalcut::FColor::Red         // Emphasis foreground
    };

    tooltip = {
        finalcut::FColor::LightGray,      // Foreground
        finalcut::FColor::DarkGray      // Background
    };

    shadow = {
        finalcut::FColor::Black,  // Foreground (only for transparent shadow)
        finalcut::FColor::DarkGray       // Background
    };

    current_element = {
        finalcut::FColor::White,      // Foreground
        finalcut::FColor::Green,      // Background
        finalcut::FColor::LightGray,  // Focused foreground
        finalcut::FColor::DarkGray,   // Focused background
        finalcut::FColor::Brown,      // Incremental search foreground
        finalcut::FColor::LightRed,   // Selected foreground
        finalcut::FColor::Green,      // Selected background
        finalcut::FColor::LightRed,   // Selected focused foreground
        finalcut::FColor::DarkGray    // Selected focused background
    };

    list = {
        finalcut::FColor::Black,      // Foreground
        finalcut::FColor::LightGray,  // Background
        finalcut::FColor::LightRed,   // Selected foreground
        finalcut::FColor::LightGray   // Selected background
    };

    label = {
        finalcut::FColor::LightGray,      // Foreground
        finalcut::FColor::Black,        // Background
        finalcut::FColor::LightGray,  // Inactive foreground
        finalcut::FColor::DarkGray,   // Inactive background
        finalcut::FColor::Red,        // Hotkey foreground
        finalcut::FColor::LightGray,  // Hotkey background
        finalcut::FColor::White,       // Emphasis foreground
        finalcut::FColor::DarkGray    // Ellipsis foreground
    };

    input_field = {
        finalcut::FColor::White,      // Foreground
        finalcut::FColor::Blue,       // Background
        finalcut::FColor::White,      // Focused foreground
        finalcut::FColor::Green,      // Focused background
        finalcut::FColor::White,      // Inactive foreground
        finalcut::FColor::BlueViolet   // Inactive background
    };

    toggle_button = {
        finalcut::FColor::White,      // Foreground
        finalcut::FColor::Blue,         // Background
        finalcut::FColor::White,      // Focused foreground
        finalcut::FColor::Green,      // Focused background
        finalcut::FColor::DarkGray,   // Inactive foreground
        finalcut::FColor::LightGray   // Inactive background
    };

    button = {
        finalcut::FColor::White,      // Foreground
        finalcut::FColor::Blue,       // Background
        finalcut::FColor::White,      // Focused foreground
        finalcut::FColor::Blue,      // Focused background
        finalcut::FColor::White,       // Inactive foreground
        finalcut::FColor::LightGray,  // Inactive background
        finalcut::FColor::LightRed         // Hotkey foreground
    };

    titlebar = {
        finalcut::FColor::White,      // Foreground
        finalcut::FColor::Blue,       // Background
        finalcut::FColor::LightGray,  // Inactive foreground
        finalcut::FColor::DarkGray,   // Inactive background
        finalcut::FColor::Black,      // Button foreground
        finalcut::FColor::LightGray,  // Button background
        finalcut::FColor::LightGray,  // Focused button foreground
        finalcut::FColor::Black       // Focused button background
    };

    menu = {
        finalcut::FColor::Black,      // Foreground
        finalcut::FColor::Yellow,     // Background
        finalcut::FColor::White,      // Focused foreground
        finalcut::FColor::Blue,       // Focused background
        finalcut::FColor::Cyan,       // Inactive foreground
        finalcut::FColor::Yellow,     // Inactive background
        finalcut::FColor::Red,        // Hotkey foreground
        finalcut::FColor::Yellow      // Hotkey background
    };

    statusbar = {
        finalcut::FColor::White,      // Foreground
        finalcut::FColor::DarkGray,   // Background
        finalcut::FColor::White,      // Focused foreground
        finalcut::FColor::Green,      // Focused background
        finalcut::FColor::Black,      // Separator foreground
        finalcut::FColor::LightRed,   // Hotkey foreground
        finalcut::FColor::DarkGray,   // Hotkey background
        finalcut::FColor::LightRed,   // Focused hotkey foreground
        finalcut::FColor::Green       // Focused hotkey background
    };

    scrollbar = {
        finalcut::FColor::Black,      // Foreground
        finalcut::FColor::Green,      // Background
        finalcut::FColor::Black,      // Button foreground
        finalcut::FColor::Green,      // Button background
        finalcut::FColor::Cyan,       // Inactive button foreground
        finalcut::FColor::LightGray   // Inactive button background
    };

    progressbar = {
        finalcut::FColor::Green,      // Foreground
        finalcut::FColor::DarkGray    // Background
    };
}
