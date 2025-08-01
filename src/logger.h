// logger.hpp
#pragma once

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


class Logger {
    Logger() = default;  // Prevent instantiation
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    static MainWindow* _mainWindow;

public:
    static bool _useTui;

    static void init(MainWindow *mainWindow, bool useTui = true, int logLevel = 1, const std::string& logger_name = "kufiBot",
                    const std::string& file_name = "/var/log/kufibot.log") {
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
