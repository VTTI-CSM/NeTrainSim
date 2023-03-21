/**
 * @file	~\NeTrainSim\src\util\Logger.h.
 *
 * Declares the logger class
 */
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <iostream>

namespace Logger {
    /** Values that represent log levels */
    enum class LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    /**
     * Level to string
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	level	The level.
     *
     * @returns	A std::string.
     */
    inline static std::string levelToString(LogLevel level) {
        switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
        }
    }

    /**
     * A logger.
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    class Logger {
    private:
        /** (Immutable) the minimum log level */
        static constexpr LogLevel minLogLevel = LogLevel::DEBUG;
        /** The log file */
        static std::ofstream logFile;

    public:

        /**
         * Logs a message
         *
         * @author	Ahmed Aredah
         * @date	2/28/2023
         *
         * @param 	level  	The level.
         * @param 	message	The message.
         */
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

        /**
         * Debug log message to console
         *
         * @author	Ahmed Aredah
         * @date	2/28/2023
         *
         * @param 	message	The message.
         */
        static void DebugLogMessageToConsole(const std::string& message) {
            if (LogLevel::DEBUG <= minLogLevel) { return; }
            std::cout << levelToString(LogLevel::DEBUG) << ": ";
            std::cout << message << std::endl;

        }
    };

}

#endif // LOGGER_H