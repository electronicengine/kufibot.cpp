// logger.hpp
#ifndef LOGGER_H
#define LOGGER_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
#include <string>
#include <iostream>
#include "tui/main_window.h"
#include <spdlog/fmt/fmt.h>
#include <source_location>
#include <regex>

#define LOG_CACHE 1000

struct CachedLog {
    std::string log;
    spdlog::level::level_enum level;
    std::string class_name;
};

class Logger {
    Logger() = default;  // Prevent instantiation
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static std::list<CachedLog> _cachedLogs;

public:
    static bool _useTui;
    static MainWindow* _mainWindow;

    static void init(MainWindow *mainWindow, bool useTui = true, int logLevel = 1, const std::string& logger_name = "kufiBot",
                    const std::string& file_name = "/var/log/kufibot.log") {

        // Truncate the logfile before initializing the logger
        std::ofstream ofs(file_name, std::ofstream::out | std::ofstream::trunc);
        if (!ofs) {
            std::cerr << "Failed to truncate log file: " << file_name << std::endl;
        }
        ofs.close();

        try {
            _mainWindow = mainWindow;
            _useTui = useTui;
            // Create rotating file sink - 5MB size, 3 rotated files

            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                file_name, 1024 * 1024 * 5, 3);

            // Create console sink
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

            // Create logger with multiple sinks
            std::vector<spdlog::sink_ptr> sinks {console_sink, file_sink};
            auto logger = std::make_shared<spdlog::logger>(logger_name, sinks.begin(), sinks.end());

            // Set as default logger
            spdlog::set_default_logger(logger);
            // Set log pattern
            spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] [%t] %v");
            // Set log level
            spdlog::set_level((spdlog::level::level_enum) logLevel);

        } catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
        }
    }

    static void print_cached_logs(std::string className) {
        for (auto it = _cachedLogs.begin(); it != _cachedLogs.end(); ) {
            if (it->class_name == className) {
                std::string full_message = fmt::format("<{}> {}", it->class_name, it->log);

                switch (it->level) {
                    case spdlog::level::trace: spdlog::trace(full_message); break;
                    case spdlog::level::debug: spdlog::debug(full_message); break;
                    case spdlog::level::info:  spdlog::info(full_message);  break;
                    case spdlog::level::warn:  spdlog::warn(full_message);  break;
                    case spdlog::level::err:   spdlog::error(full_message); break;
                    default: break;
                }

                it = _cachedLogs.erase(it); // Erase and advance iterator
            } else {
                ++it; // Skip unmatched logs
            }
        }
    }


    static inline std::string extract_class_name(const std::string& pretty_function) {
        std::regex class_regex(R"((\w+)::\w+\(.*\))");
        std::smatch match;
        if (std::regex_search(pretty_function, match, class_regex)) {
            return match[1];
        }
        return "UnknownClass";
    }

    template <class... LogStrArgs>
    static void trace(const char* function, const std::string& logStr, LogStrArgs&&... logStrArgs) {
        std::string class_name = extract_class_name(function);
        auto formattedStr = fmt::format(logStr, logStrArgs...);

        if (_cachedLogs.size() <= LOG_CACHE) {
            _cachedLogs.push_back({formattedStr, spdlog::level::trace, class_name});
        }else {
            _cachedLogs.erase(_cachedLogs.begin());
            _cachedLogs.push_back({formattedStr, spdlog::level::trace, class_name});
        }

        if (!_useTui) {
            std::string full_message = fmt::format("<{}> {}",
                                       class_name,
                                       formattedStr);
            spdlog::trace(full_message);
        }else {
            _mainWindow->log(formattedStr, LogLevel::trace, class_name);
        }
    }

    template <class... LogStrArgs>
    static void info(const char* function, const std::string& logStr, LogStrArgs&&... logStrArgs) {
        std::string class_name = extract_class_name(function);
        auto formattedStr = fmt::format(logStr, std::forward<LogStrArgs>(logStrArgs)...);

        if (_cachedLogs.size() <= LOG_CACHE) {
            _cachedLogs.push_back({formattedStr, spdlog::level::info, class_name});
        }else {
            _cachedLogs.erase(_cachedLogs.begin());
            _cachedLogs.push_back({formattedStr, spdlog::level::info, class_name});
        }

        if (!_useTui) {
            std::string full_message = fmt::format("<{}> {}", class_name, formattedStr);
            spdlog::info(full_message);
        } else {
            _mainWindow->log(formattedStr, LogLevel::info, class_name);
        }
    }

    template <class... LogStrArgs>
    static void debug(const char* function, const std::string& logStr, LogStrArgs&&... logStrArgs) {
        std::string class_name = extract_class_name(function);
        auto formattedStr = fmt::format(logStr, logStrArgs...);

        if (_cachedLogs.size() <= LOG_CACHE) {
            _cachedLogs.push_back({formattedStr, spdlog::level::debug, class_name});
        }else {
            _cachedLogs.erase(_cachedLogs.begin());
            _cachedLogs.push_back({formattedStr, spdlog::level::debug, class_name});
        }

        if (!_useTui) {
            std::string full_message = fmt::format("<{}> {}",
                                       class_name,
                                       formattedStr);
            spdlog::debug(full_message);
        }else {
            _mainWindow->log(formattedStr, LogLevel::debug, class_name);
        }
    }

    template <class... LogStrArgs>
    static void warning(const char* function, const std::string& logStr, LogStrArgs&&... logStrArgs) {
        std::string class_name = extract_class_name(function);
        auto formattedStr = fmt::format(logStr, logStrArgs...);

        if (_cachedLogs.size() <= LOG_CACHE) {
            _cachedLogs.push_back({formattedStr, spdlog::level::warn, class_name});
        }else {
            _cachedLogs.erase(_cachedLogs.begin());
            _cachedLogs.push_back({formattedStr, spdlog::level::warn, class_name});
        }

        if (!_useTui) {
            std::string full_message = fmt::format("<{}> {}",
                                       class_name,
                                       formattedStr);
            spdlog::warn(full_message);
        }else {
            _mainWindow->log(formattedStr, LogLevel::warn, class_name);
        }
    }

    template <class... LogStrArgs>
    static void error(const char* function, const std::string& logStr, LogStrArgs&&... logStrArgs) {
        std::string class_name = extract_class_name(function);
        auto formattedStr = fmt::format(logStr, logStrArgs...);

        if (_cachedLogs.size() <= LOG_CACHE) {
            _cachedLogs.push_back({formattedStr, spdlog::level::err, class_name});
        }else {
            _cachedLogs.erase(_cachedLogs.begin());
            _cachedLogs.push_back({formattedStr, spdlog::level::err, class_name});
        }

        if (!_useTui) {
            std::string full_message = fmt::format("<{}> {}",
                                       class_name,
                                       formattedStr);
            spdlog::error(full_message);
        }else {
            _mainWindow->log(formattedStr, LogLevel::error, class_name);
        }
    }

    template <class... LogStrArgs>
    static void critical(const char* function, const std::string& logStr, LogStrArgs&&... logStrArgs) {
        std::string class_name = extract_class_name(function);
        auto formattedStr = fmt::format(logStr, logStrArgs...);

        if (_cachedLogs.size() <= LOG_CACHE) {
            _cachedLogs.push_back({formattedStr, spdlog::level::critical, class_name});
        }else {
            _cachedLogs.erase(_cachedLogs.begin());
            _cachedLogs.push_back({formattedStr, spdlog::level::critical, class_name});
        }

        if (!_useTui) {
            std::string full_message = fmt::format("<{}> {}",
                                       class_name,
                                       formattedStr);
            spdlog::critical(full_message);
        }else {
            _mainWindow->log(formattedStr, LogLevel::critical, class_name);
        }
    }
};


#define TRACE(logStr, ...) Logger::trace(__PRETTY_FUNCTION__, logStr, ##__VA_ARGS__)
#define INFO(logStr, ...) Logger::info(__PRETTY_FUNCTION__, logStr, ##__VA_ARGS__)
#define DEBUG(logStr, ...) Logger::debug(__PRETTY_FUNCTION__, logStr, ##__VA_ARGS__)
#define WARNING(logStr, ...) Logger::warning(__PRETTY_FUNCTION__, logStr, ##__VA_ARGS__)
#define ERROR(logStr, ...) Logger::error(__PRETTY_FUNCTION__, logStr, ##__VA_ARGS__)
#define CRITICAL(logStr, ...) Logger::critical(__PRETTY_FUNCTION__, logStr, ##__VA_ARGS__)

#endif // LOGGER_H