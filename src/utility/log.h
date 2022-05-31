/**
 * Contains logging functionality.
 *
 * All Logs handle errors by silently ignoring them, allowing the program to
 * continue operating without their output.
 */
#pragma once

#include "util.h"
#include "config.h"
#include <sstream>
#include <fstream>
#include <streambuf>
#include <ostream>
#include <filesystem>

/**
 * The log can accept messages as strings, add a timestamp, and write it to
 * some destination defined by the derived implementation.
 */
class Log
{

public:

    Log() noexcept;
    virtual ~Log() noexcept;

    /**
     * Print a one-line log message with decorations for the current timestamp and level.
     */
    void write(Configuration::LogLevel level, const std::string& fmt, auto&&... args) noexcept
    {
        if (level_ <= level)
            writeImpl(tag(level) + format(fmt, args...) + "\n");
    }

    /**
     * Print a formatted log string without any decorations or newline.
     */
    Log& writeRaw(Configuration::LogLevel level, const std::string& fmt, auto&&... args) noexcept
    {
        if (level_ <= level)
            writeImpl(format(fmt, args...));

        return *this;
    }

    /**
     * Return the current log level.
     *
     * The log prints all messages which have a level above or at the configured
     * log level.
     */
    Configuration::LogLevel level() const noexcept;

    /**
     * Change the current log level, which determines the messages that pass through.
     */
    void setLevel(Configuration::LogLevel level) noexcept;

protected:

    virtual void writeImpl(const std::string& item) noexcept = 0;

private:

    std::string tag(Configuration::LogLevel level) const noexcept;

    Configuration::LogLevel level_;

};

/**
 * This log keeps all log messages in memory.
 *
 * It acts as a buffer for the startup phase of the program, during which the
 * log is not yet initialized according to the program options.
 */
class MemoryLog : public Log
{

public:

    virtual ~MemoryLog() noexcept;

    /**
     * After setting up the real log, replay this log's memory as a
     * handover mechanism. Afterwards, behave like a @c StreamLog
     * targeted at std::clog
     */
    void replay(Log& successor) noexcept;

    /**
     * Dump this log's memory to std::clog and afterwards behave like
     * a @c StreamLog targeted at std::clog.
     */
    void shutdown() noexcept;

protected:

    virtual void writeImpl(const std::string& item) noexcept override;

private:

    struct Item
    {
        Configuration::LogLevel level;
        std::string contents;
    };

    std::vector<Item> memory_;
    bool shutdown_;

};

/**
 * This log writes all log messages to a file.
 */
class FileLog : public Log
{

public:

    /**
     * Construct a file log which appends to the file at the given path.
     */
    explicit FileLog(const std::filesystem::path& filepath) noexcept;

protected:

    virtual void writeImpl(const std::string& item) noexcept override;

private:

    std::ofstream file_;

};

/**
 * This log writes all log messages to a specified stream.
 */
class StreamLog : public Log
{

public:

    /**
     * Construct a log which writes to the given stream.
     */
    explicit StreamLog(std::ostream& stream) noexcept;

protected:

    virtual void writeImpl(const std::string& item) noexcept override;

private:

    std::ostream* stream_;

};

/**
 * This log forwards all log messages to two other logs.
 */
class DuplicateLog : public Log
{

public:

    /**
     * Construct a log which forwards to the given first and second logs.
     */
    explicit DuplicateLog(Log& first, Log& second) noexcept;

    Log& first() const noexcept;
    Log& second() const noexcept;

protected:

    virtual void writeImpl(const std::string& item) noexcept override;

private:

    Log* first_;
    Log* second_;

};

extern MemoryLog stage1log; // initial log until configuration is established
extern Log* theLog; // global log object

/**
 * Logging convenience function which supports string formatting.
 */
void error(const std::string& fmt, auto&&... args) noexcept
{
    theLog->write(Configuration::LogLevel::ERROR, fmt, args...);
}

/**
 * Logging convenience function which supports string formatting.
 */
void info(const std::string& fmt, auto&&... args) noexcept
{
    theLog->write(Configuration::LogLevel::INFO, fmt, args...);
}

/**
 * Logging convenience function which supports string formatting.
 */
void trace(const std::string& fmt, auto&&... args) noexcept
{
    theLog->write(Configuration::LogLevel::TRACE, fmt, args...);
}
