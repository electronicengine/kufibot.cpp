// logger.hpp
#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
#include <string>
#include <iostream>
#include "tui/main_window.h"

class Logger {
public:
    static void init(const std::string& logger_name = "kufiBot",
                    const std::string& file_name = "/var/log/kufibot.log") {
        try {
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
            spdlog::set_level(spdlog::level::debug);

        } catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
        }
    }

    static void trace(const std::string& message) {
        spdlog::trace(message);
        MainWindow::log(message, LogLevel::LOG_TRACE);
    }

    static void debug(const std::string& message) {
        spdlog::debug(message);
        MainWindow::log(message, LogLevel::LOG_INFO);
    }

    static void info(const std::string& message) {
        spdlog::info(message);
        MainWindow::log(message, LogLevel::LOG_INFO);
    }

    static void warn(const std::string& message) {
        spdlog::warn(message);
        MainWindow::log(message, LogLevel::LOG_WARNING);
    }

    static void error(const std::string& message) {
        spdlog::error(message);
        MainWindow::log(message, LogLevel::LOG_ERROR);
    }

    static void critical(const std::string& message) {
        spdlog::critical(message);
        MainWindow::log(message, LogLevel::LOG_WARNING);
    }

    // Template versions for logging with formatting
    template<typename... Args>
    static void trace(fmt::format_string<Args...> fmt, Args&&... args) {
        spdlog::trace(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void debug(fmt::format_string<Args...> fmt, Args&&... args) {
        spdlog::debug(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void info(fmt::format_string<Args...> fmt, Args&&... args) {
        spdlog::info(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void warn(fmt::format_string<Args...> fmt, Args&&... args) {
        spdlog::warn(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void error(fmt::format_string<Args...> fmt, Args&&... args) {
        spdlog::error(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void critical(fmt::format_string<Args...> fmt, Args&&... args) {
        spdlog::critical(fmt, std::forward<Args>(args)...);
    }

private:
    Logger() = default;  // Prevent instantiation
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};