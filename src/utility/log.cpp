#include "log.h"
#include <chrono>
#include <ctime>
#include <string>
#include <ios>

Log::Log() noexcept
    : level_(Configuration::LogLevel::TRACE)
{
}

Log::~Log() noexcept = default;

void Log::write(const std::string& line, Configuration::LogLevel level) noexcept
{
    if (level_ <= level) {
        std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::string nowstr(30, '\0');
        std::strftime(&nowstr[0], nowstr.size(), "%Y-%m-%d %H:%M:%S ", std::localtime(&now));

        using namespace std::string_literals;
        std::string levels[] = { "[ERROR] "s, "[INFO] "s, "[TRACE] "s };
        std::string levelstr = levels[static_cast<std::size_t>(level)];

        writeImpl(nowstr + levelstr + line + "\n");
    }
}

Configuration::LogLevel Log::level() const noexcept
{
    return level_;
}

void Log::setLevel(Configuration::LogLevel level) noexcept
{
    level_ = level;
}


void MemoryLog::replay(Log& successor) noexcept
{
    // choose highest level to make sure the messages are written
    successor.writeRaw(contents_.str(), Configuration::LogLevel::ERROR);
}

void MemoryLog::writeImpl(const std::string& line) noexcept
{
    contents_ << line;
}

std::ostream& MemoryLog::stream() noexcept
{
    return contents_;
}


FileLog::FileLog(const std::filesystem::path& filepath) noexcept
    : file_(filepath, std::ios::app)
{
}

void FileLog::writeImpl(const std::string& line) noexcept
{
    file_ << line;
}

std::ostream& FileLog::stream() noexcept
{
    return file_;
}


StreamLog::StreamLog(std::ostream& stream) noexcept
    : stream_(&stream)
{
}

void StreamLog::writeImpl(const std::string& line) noexcept
{
    *stream_ << line;
}

std::ostream& StreamLog::stream() noexcept
{
    return *stream_;
}


DuplicateLog::DuplicateLog(Log& first, Log& second) noexcept
    : std::ostream(this), first_(&first), second_(&second)
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

int DuplicateLog::overflow(int c) noexcept
{
    writeImpl(std::string{ static_cast<char>(c) });
    return 0;
}

void DuplicateLog::writeImpl(const std::string& line) noexcept
{
    // choose highest level to make sure the messages are written
    first_->writeRaw(line, Configuration::LogLevel::ERROR);
    second_->writeRaw(line, Configuration::LogLevel::ERROR);
}

std::ostream& DuplicateLog::stream() noexcept
{
    return *this;
}
