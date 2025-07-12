#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "final/final.h"
#include <utility>
#include <deque>
#include "graph_window.h"
#include "compass_rt_graph_window.h"
#include "body_controller_window.h"
#include "servo_controller_window.h"
#include "measurements_window.h"
#include "chat_window.h"

#define LOG_HISTORY_SIZE    2000

using namespace std;
using namespace finalcut;

enum class LogLevel : uint_fast8_t {
  LOG_TRACE = 0,
  LOG_INFO,
  LOG_WARNING,
  LOG_ERROR
};


class MainWindow final : public finalcut::FDialog
{
  public:
    // Constructor
    explicit MainWindow (finalcut::FWidget* = nullptr);

    static void log(const std::string& logLine, LogLevel logLevel);
    // Destructor
    ~MainWindow() override;

  protected:
    struct LogItem {
      LogLevel logLevel;
      std::wstring logString;
    };
    using LogList = std::list<LogItem>;

    void onTimer (finalcut::FTimerEvent*) override;

  private:
    static bool _noTui;
    static std::mutex _logMtx;
    static std::mutex _logViewMtx;
    static std::mutex _logFilterMtx;
    static std::mutex _logLevelMtx;
    static std::mutex _quitCbMtx;
    static std::mutex _autoScrollMtx;
    static std::mutex _searchStringMtx;
    static std::mutex _fileMenuMtx;
    static std::mutex _quitMtx;
    LogLevel _currentLogLevel{LogLevel::LOG_INFO};
    static std::deque<std::pair<std::string, LogLevel>> _logHistory;
    static std::mutex _loggerViewMtx;

    bool _autoScroll{true};
    std::wstring _searchString;
    std::function<void(void)> _quitCb;

    CompassRtGraphWindow *_compassRTGraphWindow;
    BodyControllerWindow *_bodyControllerWindow;
    ServoControllerWindow *_servoControllerWindow;
    MeasurementsWindow *_measurementsWindow;
    ChatWindow *_chatWindow;

    finalcut::FTextView _textView{this};
    finalcut::FButtonGroup _radiobutton_group {L"Log Level", this};
    finalcut::FButtonGroup _toggleGroup {L"Auto Scroll", this};
    finalcut::FSwitch _toggleAutoScroll{&_toggleGroup};

    finalcut::FLineEdit _lineEditFilter {this};
    finalcut::FRadioButton _error {"E", &_radiobutton_group};
    finalcut::FRadioButton _warning {"W", &_radiobutton_group};
    finalcut::FRadioButton _info {"I", &_radiobutton_group};
    finalcut::FRadioButton _trace {"T", &_radiobutton_group};

    finalcut::FMenuBar        _menuBar{this};
    finalcut::FMenu           _windowsMenu{"Windows", &_menuBar};
    finalcut::FMenuItem       _quit{"&Quit", &_menuBar};
    finalcut::FStatusBar      _statusBar{this};
    finalcut::FMenuItem       _compassRTGraphWindowMenuButton{"&Open Compass RT Graph", &_windowsMenu};
    finalcut::FMenuItem       _bodyControllerWindowMenuButton{"&Open Body Controller Window", &_windowsMenu};
    finalcut::FMenuItem       _servoControllerWindowMenuButton{"&Open Servo Controller Window", &_windowsMenu};
    finalcut::FMenuItem       _measurementsWindowMenuButton{"&Open Measurements Window", &_windowsMenu};
    finalcut::FMenuItem       _chatWindowMenuButton{"&Open Chat Window", &_windowsMenu};

    void append_log_view(const std::string& logLine, LogLevel logLevel);
    void configure_file_nenu_items();
    void activate_window (finalcut::FDialog*) const;
    void text_view_scroll_up(void);

    void initLayout() override;
    template <typename InstanceT
            , typename CallbackT
            , typename... Args>
    void add_clicked_callback ( finalcut::FWidget*
                            , InstanceT&&, CallbackT&&, Args&&... );

    void onClose (finalcut::FCloseEvent*) override;
    void show_compass_rt_graph_window();
    void show_body_controller_window();
    void show_servo_controller_window();
    void show_measurements_window();
    void show_chat_window();

    void auto_scroll_toggle();

    virtual void log_level_clicked(LogLevel){}
    virtual void log_filter_changed(void){};

    void filter(const LogList&);
    void print_with_search(const LogItem&);

   

};

#endif