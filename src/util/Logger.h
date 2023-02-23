#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <iostream>

namespace Logger {
    enum class LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    inline static std::string levelToString(LogLevel level) {
        switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
        }
    }

    class Logger {
    private:
        static constexpr LogLevel minLogLevel = LogLevel::DEBUG;
        static std::ofstream logFile;

    public:
        static void logMessage(LogLevel level, const std::string& message) {
            if (level <= minLogLevel) { return; }
            if (!logFile.is_open()) {
                std::cerr << "Error opening log file" << std::endl;
                return;
            }

            auto now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            logFile << std::put_time(std::localtime(&now_c), "[%F %T]") << " ";
            logFile << levelToString(level) << ": ";
            logFile << message << std::endl;
        }

        static void DebugLogMessageToConsole(const std::string& message) {
            if (LogLevel::DEBUG <= minLogLevel) { return; }
            std::cout << levelToString(LogLevel::DEBUG) << ": ";
            std::cout << message << std::endl;

        }
    };

}

#endif // LOGGER_H