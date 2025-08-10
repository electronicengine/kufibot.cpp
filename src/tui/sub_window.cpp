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

#include "sub_window.h"

SubWindow::SubWindow(finalcut::FWidget *parent) : FDialog(parent)
{
}

SubWindow::~SubWindow() noexcept
{
}

void SubWindow::onClose(finalcut::FCloseEvent *)
{
    hide();
    auto  parent = getParent();
    activate_window((FDialog*) parent);
}

void SubWindow::onShow(finalcut::FShowEvent *)
{
    activate_window(this);
}

void SubWindow::adjustSize()
{
      finalcut::FDialog::adjustSize();
}

void SubWindow::activate_window(finalcut::FDialog *win) const
{
    if ( ! win || win->isWindowHidden() || win->isWindowActive() )
        return;

    const bool has_raised = finalcut::FWindow::raiseWindow(win);
    win->activateDialog();

    if ( has_raised )
        win->redraw();
}

