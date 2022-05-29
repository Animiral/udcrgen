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
    void write(const std::string& line, Configuration::LogLevel level) noexcept;

    /**
     * Print any value that is compatible with @c std::ostream without decorations.
     *
     * The user is responsible for ending with a newline before the next @c write.
     */
    template<typename T>
    Log& writeRaw(const T& item, Configuration::LogLevel level) noexcept
    {
        if (level_ <= level)
            stream() << item;

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

    virtual void writeImpl(const std::string& line) noexcept = 0;
    virtual std::ostream& stream() noexcept = 0;

private:

    Configuration::LogLevel level_;

};

/**
 * This log keeps all log messages in memory.
 *
 * It acts as a buffer for the startup phase of the program, during which the
 * log is not yet initialized according to the program options.
 *
 * After setting up the real log, this log can @c replay its contents as a
 * handover mechanism.
 *
 * As an exception to the base Log interface, @c writeRaw is only supported for strings.
 */
class MemoryLog : public Log
{

public:

    void replay(Log& successor) noexcept;

protected:

    virtual void writeImpl(const std::string& line) noexcept override;
    virtual std::ostream& stream() noexcept override;

private:

    std::ostringstream contents_;

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

    virtual void writeImpl(const std::string& line) noexcept override;
    virtual std::ostream& stream() noexcept override;

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

    virtual void writeImpl(const std::string& line) noexcept override;
    virtual std::ostream& stream() noexcept override;

private:

    std::ostream* stream_;

};

/**
 * This log forwards all log messages to two other logs.
 */
class DuplicateLog : public Log, private std::streambuf, public std::ostream
{

public:

    /**
     * Construct a log which forwards to the given first and second logs.
     */
    explicit DuplicateLog(Log& first, Log& second) noexcept;

    Log& first() const noexcept;
    Log& second() const noexcept;

    int overflow(int c) noexcept override;

protected:

    virtual void writeImpl(const std::string& line) noexcept override;
    virtual std::ostream& stream() noexcept override;

private:

    Log* first_;
    Log* second_;

};

extern MemoryLog stage1log; // initial log until configuration is established
extern Log* theLog; // global log object

/**
 * Logging convenience function which supports string formatting.
 */
template<typename ...Args>
void error(const std::string& fmt, Args... args) noexcept
{
    theLog->write(format(fmt, args...), Configuration::LogLevel::ERROR);
}

/**
 * Logging convenience function which supports string formatting.
 */
template<typename ...Args>
void info(const std::string& fmt, Args... args) noexcept
{
    theLog->write(format(fmt, args...), Configuration::LogLevel::INFO);
}

/**
 * Logging convenience function which supports string formatting.
 */
template<typename ...Args>
void trace(const std::string& fmt, Args... args) noexcept
{
    theLog->write(format(fmt, args...), Configuration::LogLevel::TRACE);
}
