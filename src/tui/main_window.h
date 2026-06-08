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
#include "spdlog/spdlog.h"
#include <chrono>
#include <optional>
#include <tuple>

#define LOG_HISTORY_SIZE    2000

using namespace std;
using namespace finalcut;

enum class LogLevel {
  trace,
  info,
  debug,
  warn,
  error,
  critical
};

const inline std::map<LogLevel, std::string> Log_Level_Strings = {{LogLevel::trace, "trace"},
                                                                  {LogLevel::info, "info "},
                                                                  {LogLevel::debug, "debug"},
                                                                  {LogLevel::warn, "warn."},
                                                                  {LogLevel::error, "error"},
                                                                  {LogLevel::critical, "crit."}};

class MainWindow final : public finalcut::FDialog
{
  public:
    // Constructor
    // Destructor
  explicit MainWindow (finalcut::FWidget* = nullptr);
  ~MainWindow() override;

  void log(const std::string& logLine, LogLevel logLevel, const std::string& className = "");
  void queue_sensor_data(const SensorData& sensorData);
  void queue_llm_response(const std::string& response);
  void queue_compass_direction(int angle);
  void queue_servo_joints(const std::map<ServoMotorJoint, uint8_t>& jointAngles);
  void set_sensor_refresh_callback(std::function<void()> callback);

  CompassRtGraphWindow *_compassRTGraphWindow;
  BodyControllerWindow *_bodyControllerWindow;
  ServoControllerWindow *_servoControllerWindow;
  MeasurementsWindow *_measurementsWindow;
  ChatWindow *_chatWindow;

  protected:
    struct LogItem {
      LogLevel logLevel;
      std::wstring logString;
    };
    using LogList = std::list<LogItem>;

  private:
    static bool _noTui;
    std::mutex _loggerViewMtx;

    bool _autoScroll{true};
    std::wstring _searchString;
    std::function<void(void)> _quitCb;


    FApplication *_app;
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

    void configure_file_nenu_items();
    void activate_window (finalcut::FDialog*) const;
    void text_view_scroll_up(void);

    void initLayout() override;
    void onTimer (finalcut::FTimerEvent*) override;
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
    void append_log_line(const std::string& logLine, LogLevel logLevel, const std::string& className);

    std::mutex _pendingUiMtx;
    std::deque<std::tuple<std::string, LogLevel, std::string>> _pendingLogs;
    std::deque<std::string> _pendingLlmResponses;
    std::optional<SensorData> _pendingSensorData;
    std::optional<int> _pendingCompassDirection;
    std::optional<std::map<ServoMotorJoint, uint8_t>> _pendingServoJointAngles;
    std::function<void()> _sensorRefreshCallback;
    std::chrono::steady_clock::time_point _lastSensorRefreshTime{std::chrono::steady_clock::now()};

};

#endif