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

#include "main_window.h"
#include "../logger.h"

bool MainWindow::_noTui = true;

MainWindow::MainWindow (finalcut::FWidget* parent)
  : finalcut::FDialog{parent}
{
    _noTui = true;
    _textView.setGeometry(FPoint{2, 4}, FSize{FVTerm::getFOutput()->getColumnNumber() - 4, FVTerm::getFOutput()->getLineNumber() - 8});
    _textView.addCallback("mouse-wheel-up", this, &MainWindow::text_view_scroll_up);

    _lineEditFilter.setLabelText(L"Filter");
    _lineEditFilter.setLabelOrientation(finalcut::FLineEdit::LabelOrientation::Above);
    _lineEditFilter.setGeometry(finalcut::FPoint{3,2}, finalcut::FSize{12, 1});

    _radiobutton_group.setGeometry(finalcut::FPoint{18,1}, finalcut::FSize{25, 3});
    _error.setGeometry(finalcut::FPoint{1,1}, finalcut::FSize{5, 1});
    _warning.setGeometry(finalcut::FPoint{7,1}, finalcut::FSize{5, 1});
    _info.setGeometry(finalcut::FPoint{13,1}, finalcut::FSize{5, 1});
    _trace.setGeometry(finalcut::FPoint{19,1}, finalcut::FSize{5, 1});
    _info.setChecked();

    _toggleGroup.setGeometry(FPoint{44, 1}, FSize{15, 3});
    _toggleAutoScroll.setGeometry(finalcut::FPoint{2,1}, finalcut::FSize{12, 1});
    _toggleAutoScroll.setChecked(true);
    _toggleAutoScroll.addCallback("toggled", this, &MainWindow::auto_scroll_toggle);

    _compassRTGraphWindow = new CompassRtGraphWindow{this, -200, 200, -200, 200};
    _compassRTGraphWindow->setGeometry (FPoint{8, 16}, FSize{60, 30});
    _compassRTGraphWindow->setMinimumSize (FSize{30, 15});
    _compassRTGraphWindow->setResizeable();
    _compassRTGraphWindow->setMinimizable();
    _compassRTGraphWindow->hide();

    _bodyControllerWindow = new BodyControllerWindow{this};
    _bodyControllerWindow->setMinimumSize (FSize{30, 20});
    _bodyControllerWindow->setResizeable();
    _bodyControllerWindow->setMinimizable();
    _bodyControllerWindow->hide();

    _servoControllerWindow = new ServoControllerWindow{this};
    _servoControllerWindow->setMinimumSize (FSize{30, 30});
    _servoControllerWindow->setResizeable();
    _servoControllerWindow->setMinimizable();
    _servoControllerWindow->hide();

    _measurementsWindow = new MeasurementsWindow{this};
    _measurementsWindow->setMinimumSize (FSize{30, 18});
    _measurementsWindow->setResizeable();
    _measurementsWindow->setMinimizable();
    _measurementsWindow->hide();

    _chatWindow = new ChatWindow{this};
    _chatWindow->setMinimumSize (FSize{60, 35});
    _chatWindow->setResizeable();
    _chatWindow->setMinimizable();
    _chatWindow->hide();

    setTitlebarButtonVisibility(false);
    configure_file_nenu_items();

}

MainWindow::~MainWindow()
{

}


void MainWindow::log(const std::string &logLine, LogLevel logLevel, const std::string &className)
{
    std::lock_guard<std::mutex> lock(_loggerViewMtx);

    auto now = std::chrono::high_resolution_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now());

    thread_local std::tm local_tm;
    local_tm = *std::localtime(&time_t_now);

    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::string full_log_line;
    full_log_line.reserve(256); // Adjust based on typical log line length

    std::ostringstream oss;
    oss << '[' << std::put_time(&local_tm, "%H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << milliseconds.count() << ']'
        << " [" << Log_Level_Strings.at(logLevel) << ']'
        << " [" << syscall(SYS_gettid) << ']'
        << " <" << className << "> "
        << logLine;

    full_log_line = oss.str();

    _textView.append(full_log_line);

     auto lineIndex = _textView.getLines().size() - 1;
     finalcut::FColorPair colorPair;

     switch (logLevel) {
         case LogLevel::trace:
             colorPair = FColorPair{finalcut::FColor::White, finalcut::FColor::Black};
             break;
         case LogLevel::info:
             colorPair = FColorPair{finalcut::FColor::Green, finalcut::FColor::Black};
             break;
         case LogLevel::debug:
             colorPair = FColorPair{finalcut::FColor::Blue, finalcut::FColor::Black};
             break;
         case LogLevel::warn:
             colorPair = FColorPair{finalcut::FColor::Orange1, finalcut::FColor::Black};
             break;
         case LogLevel::error:
             colorPair = FColorPair{finalcut::FColor::Red, finalcut::FColor::Black};
             break;
         case LogLevel::critical:
             colorPair = FColorPair{finalcut::FColor::DarkRed2, finalcut::FColor::Black};
             break;
         default:
             colorPair = FColorPair{finalcut::FColor::Magenta, finalcut::FColor::Black};
             break;
     }
    _textView.addHighlight(lineIndex, finalcut::FTextView::FTextHighlight{16, 5, colorPair});

    if(_autoScroll)
        _textView.scrollToEnd();
    _textView.redraw();

}


void MainWindow::configure_file_nenu_items()
{
    _compassRTGraphWindowMenuButton.setStatusbarMessage ("Open Compass Graph Window");
    _quit.addAccelerator (finalcut::FKey::Q);  
    _quit.setStatusbarMessage ("Exit the program");

    add_clicked_callback (&_compassRTGraphWindowMenuButton, this, &MainWindow::show_compass_rt_graph_window);
    add_clicked_callback (&_bodyControllerWindowMenuButton, this, &MainWindow::show_body_controller_window);
    add_clicked_callback (&_servoControllerWindowMenuButton, this, &MainWindow::show_servo_controller_window);
    add_clicked_callback (&_measurementsWindowMenuButton, this, &MainWindow::show_measurements_window);
    add_clicked_callback (&_chatWindowMenuButton, this, &MainWindow::show_chat_window);

    add_clicked_callback ( &_quit, finalcut::getFApplication(), &finalcut::FApplication::cb_exitApp, this );
}

void MainWindow::auto_scroll_toggle() {
    _autoScroll = _toggleAutoScroll.isChecked();
}

void MainWindow::filter(const LogList &)
{
}

void MainWindow::print_with_search(const LogItem &)
{
}

void MainWindow::text_view_scroll_up(void) {
    if(_autoScroll){
        _autoScroll = false;
    }
}


void MainWindow::activate_window (finalcut::FDialog* win) const
{
  if ( ! win || win->isWindowHidden() || win->isWindowActive() )
    return;

  const bool has_raised = finalcut::FWindow::raiseWindow(win);
  win->activateDialog();

  if ( has_raised )
    win->redraw();
}

//----------------------------------------------------------------------
void MainWindow::initLayout()
{
  FDialog::initLayout();
}


//----------------------------------------------------------------------
template <typename InstanceT
        , typename CallbackT
        , typename... Args>
void MainWindow::add_clicked_callback ( finalcut::FWidget* widget
                                , InstanceT&& instance
                                , CallbackT&& callback
                                , Args&&... args )
{
  widget->addCallback
  (
    "clicked",
    std::bind ( std::forward<CallbackT>(callback)
              , std::forward<InstanceT>(instance)
              , std::forward<Args>(args)... )
  );
}


void MainWindow::onClose (finalcut::FCloseEvent* ev)
{
  finalcut::FApplication::closeConfirmationDialog (this, ev);
}


void MainWindow::show_compass_rt_graph_window()
{
    _compassRTGraphWindow->show();
    _compassRTGraphWindow->zoomWindow();
}

void MainWindow::show_body_controller_window()
{
    _bodyControllerWindow->show();
    _bodyControllerWindow->zoomWindow();
}

void MainWindow::show_servo_controller_window()
{
    _servoControllerWindow->show();
    _servoControllerWindow->zoomWindow();
}

void MainWindow::show_measurements_window()
{
    _measurementsWindow->show();
    _measurementsWindow->zoomWindow();
}

void MainWindow::show_chat_window()
{
    _chatWindow->show();
    _chatWindow->zoomWindow();
}
