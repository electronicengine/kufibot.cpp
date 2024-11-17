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
