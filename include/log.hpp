//
// Created by grant on 11/19/20.
//

#pragma once

#ifndef NEWTONIAN_FOOTBALL_2D_LOG_HPP
#define NEWTONIAN_FOOTBALL_2D_LOG_HPP


#include "config.hpp"

#include <iomanip>
#include <iostream>
#include <string>

#include <fmt/format.h>
#include <fmt/chrono.h>
#include <sys/stat.h>

namespace stms {

    /// The level of severity of the log message. Can be used as a bit flag.
    enum class LogLevel {
        eInvalid = 0, //!< Invalid log level. Has value 0
        eTrace = 0b1, //!< Trace info. Value 1. (first bit)
        eDebug = 0b10, //!< Debug message. Value 2. (2nd bit)
        eInfo = 0b100, //!< General info message. Value 4 (3rd bit)
        eWarn = 0b1000, //!< Warning message. Value 8. (4th bit)
        eError = 0b10000, //!< Error message. Value 16. (5th bit)
        eFatal = 0b100000, //!< Fatal error! Value 32. (6th bit)
    };

    /// Struct representing a single log message.
    struct LogRecord {
        /**
         * @brief Construct a log message record
         * @param lvl Level of severity of the message
         * @param time Time at which the message is generated
         * @param file File of origin of the log message (i.e. `main.cpp`)
         * @param line The line of the source file form which the message was emitted.
         */
        LogRecord(LogLevel lvl, std::chrono::system_clock::time_point time, const char *file, unsigned line);

        LogLevel level = LogLevel::eInvalid; //!< Severity of the message. See `LogLevel`.
        std::chrono::system_clock::time_point time; //!< Time at which the message was generated

        const char *file = ""; //!< File from which the message originated
        unsigned line{}; //!< Line of the source file from which the message originated

        fmt::memory_buffer msg; //!< Internal memory buffer to format the actual log message into. Implementation detail
    };

    void initLogging(); //!< Init STMS logging module. If you wish to init everything, use `stms::initAll` instead

    void quitLogging(); //!< Quit logging. If you used `stms::initAll`, you do not have to call this.

#   ifdef ENABLE_LOGGING
    void insertImpl(std::unique_ptr<LogRecord> rec); //!< Internal implementation detail. Don't touch.

    /**
     * @brief NEVER this function directly. Instead, use the logging macros (`INFO`, `WARN`, etc.).
     *        This function inserts a `LogRecord` into `logQueue` and starts a log-consume task (`consumeLogs`).
     *        The consume task would be asynchronous if `logPool` is not `nullptr`.
     * @tparam Args Template param allowing fmtlib arguments to be passed in
     * @param lvl Severity of the message. See `LogLevel`.
     * @param line Line of the source file the message is from
     * @param file Source file the message is from (e.g. `main.cpp`)
     * @param fmtStr String to format, the actual un-formatted log message.
     * @param args fmtlib formatting arguments.
     */
    template<typename... Args>
    void insertLog(LogLevel lvl, unsigned line, const char *file, const char *fmtStr, const Args &... args) {
        std::unique_ptr<LogRecord> insert = std::make_unique<LogRecord>(lvl, std::chrono::system_clock::now(), file, line);

        try {
            fmt::format_to(insert->msg, fmtStr, args...);
        } catch (fmt::format_error &e) {
            fmt::format_to(insert->msg,
                           "{}\t\u001b[1m\u001b[31m!<<< FORMAT ERROR: {}\u001b[0m", fmtStr, e.what());  // screw it
        }

        insertImpl(std::move(insert));
    }

    /// Logging macro for `LogLevel` of `eTrace`. Used like a fmtlib function. `TRACE("{1}, {0}!", "World", "Hello");`
#   define TRACE(...)  ::stms::insertLog(::stms::LogLevel::eTrace, __LINE__, __FILE__, __VA_ARGS__)
    /// Logging macro for `LogLevel` of `eDebug`. Used like a fmtlib function. `DEBUG("{1}, {0}!", "World", "Hello");`
#   define DEBUG(...)  ::stms::insertLog(::stms::LogLevel::eDebug, __LINE__, __FILE__, __VA_ARGS__)
    /// Logging macro for `LogLevel` of `eInfo`. Used like a fmtlib function. `INFO("{1}, {0}!", "World", "Hello");`
#   define INFO(...)   ::stms::insertLog(::stms::LogLevel::eInfo, __LINE__, __FILE__, __VA_ARGS__)
    /// Logging macro for `LogLevel` of `eWarn`. Used like a fmtlib function. `WARN("{1}, {0}!", "World", "Hello");`
#   define WARN(...)   ::stms::insertLog(::stms::LogLevel::eWarn, __LINE__, __FILE__, __VA_ARGS__)
    /// Logging macro for `LogLevel` of `eError`. Used like a fmtlib function. `ERROR("{1}, {0}!", "World", "Hello");`
#   define ERROR(...)  ::stms::insertLog(::stms::LogLevel::eError, __LINE__, __FILE__, __VA_ARGS__)
    /// Logging macro for `LogLevel` of `eFatal`. Used like a fmtlib function. `FATAL("{1}, {0}!", "World", "Hello");`
#   define FATAL(...)  ::stms::insertLog(::stms::LogLevel::eFatal, __LINE__, __FILE__, __VA_ARGS__)
#   else
    #   define TRACE(...)
#   define DEBUG(...)
#   define INFO(...)
#   define WARN(...)
#   define ERROR(...)
#   define FATAL(...)
#   endif

    class ThreadPool;
    /**
     * @brief `ThreadPool` to be used for processing log messages from `logQueue`. You can modify this variable.
     *         If set to `nullptr`, asynchronous logging will be disabled. Otherwise, log-consume tasks will be
     *         submitted to this pool. By default, this is `nullptr` and async logging is disabled.
     */
    inline ThreadPool *&getLogPool() {
        static ThreadPool *val = nullptr;
        return val;
    }

    /**
     * @brief List of hooks to call for each log message processed, in order. You can modify this variable.
     *
     * The first argument (`LogRecord *`) is the raw `LogRecord`, which contains info such as the time, file,
     * line, and level of the log message. You can modify this.
     *
     * The second argument (`std::string *`) is the full formatted string of the log message.
     * You could print this string directly. You can modify this.
     * (example: `[9:30:15.125000000   ] [   file:///home/ubuntu/MyProject/main.cpp  ] [ info ]: Hello World!`)
     *
     * By default, 1 hook is guaranteed to be registered. It is either the first or second hook, depending
     * on `stms::logToStdout`. This hook simply removes the text format control characters from the second
     * argument for printing to files. (e.g. `\u001b[1m\u001b[31mERROR[0m` would be turned into `ERROR`)
     *
     * If `stms::logToStdout` is true, the first hook
     * would be a hook that just prints the second argument to stdout.
     *
     * If `stms::logToLatestLog` is true, the next hook would be one that writes the second argument
     * to `latest.log` without flushing the output with `fflush`
     *
     * If `stms::logToUniqueFile` is true, the next hook would be one that writes the second argument to
     * `${stms::logsDir}/${DATE_TIME}.log`
     */
    inline std::vector<std::function<void(LogRecord *, std::string *)>> &getLogHooks() {
        static std::vector<std::function<void(LogRecord *, std::string *)>> val;
        return val;
    }

    void consumeLogs(); //!< Process all the logs in `logQueue`, essentially flushing the log message backlog

}

#endif //NEWTONIAN_FOOTBALL_2D_LOG_HPP
