#include "log.h"
#include <chrono>
#include <ctime>
#include <string>
#include <iostream>
#include <ios>

Log::Log() noexcept
    : level_(Configuration::LogLevel::TRACE)
{
}

Log::~Log() noexcept = default;

Configuration::LogLevel Log::level() const noexcept
{
    return level_;
}

void Log::setLevel(Configuration::LogLevel level) noexcept
{
    level_ = level;
}

std::string Log::tag(Configuration::LogLevel level) const noexcept
{
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string nowstr(30, '\0');
    std::size_t written = std::strftime(&nowstr[0], nowstr.size(), "%Y-%m-%d %H:%M:%S ", std::localtime(&now));

    using namespace std::string_literals;
    std::string levels[] = { "[!!] "s, "[ERROR] "s, "[INFO] "s, "[TRACE] "s };
    std::string levelstr = levels[static_cast<std::size_t>(level)];

    return nowstr.substr(0, written) + levelstr;
}


MemoryLog::~MemoryLog() noexcept
{
    shutdown();
}

void MemoryLog::replay(Log& successor) noexcept
{
    for (const Item& item : memory_)
        successor.writeRaw(item.level, item.contents);

    memory_.resize(0);
    shutdown_ = true;
}

void MemoryLog::shutdown() noexcept
{
    for (const Item& item : memory_) {
        if (level() <= item.level)
            std::clog << item.contents;
    }

    memory_.resize(0);
    shutdown_ = true;
}

void MemoryLog::writeImpl(const std::string& item) noexcept
{
    if (shutdown_) {
        std::clog << item;
    }
    else {
        Configuration::LogLevel level; // stupid reconstruction of message level

        if (item.npos != item.find("[INFO]"))
            level = Configuration::LogLevel::INFO;
        else if (item.npos != item.find("[TRACE]"))
            level = Configuration::LogLevel::TRACE;
        else
            level = Configuration::LogLevel::ERROR;

        memory_.push_back({ level, item });
    }
}


FileLog::FileLog(const std::filesystem::path& filepath) noexcept
    : file_(filepath, std::ios::app)
{
}

void FileLog::writeImpl(const std::string& item) noexcept
{
    file_ << item;
}


StreamLog::StreamLog(std::ostream& stream) noexcept
    : stream_(&stream)
{
}

void StreamLog::writeImpl(const std::string& item) noexcept
{
    *stream_ << item;
}


DuplicateLog::DuplicateLog(Log& first, Log& second) noexcept
    : first_(&first), second_(&second)
{
}

Log& DuplicateLog::first() const noexcept
{
    return *first_;
}

Log& DuplicateLog::second() const noexcept
{
    return *second_;
}

void DuplicateLog::writeImpl(const std::string& item) noexcept
{
    // choose highest level to make sure the messages are written
    first_->writeRaw(Configuration::LogLevel::ERROR, item);
    second_->writeRaw(Configuration::LogLevel::ERROR, item);
}


MemoryLog stage1log; // initial log until configuration is established
Log* theLog = &stage1log; // global log object
