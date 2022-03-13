#include "exception.h"

Exception::Exception()
    : message_("Unknown failure")
{
}

Exception::Exception(const std::string& fmt, const std::exception* cause)
    : message_(fmt), causeMessage_()
{
    if (cause) {
        causeMessage_ = "\n\tcaused by ";

        auto derivedCause = dynamic_cast<const Exception*>(cause);
        if (derivedCause) {
            causeMessage_.append(derivedCause->fullMessage());
        }
        else {
            causeMessage_.append("Exception: ").append(cause->what());
        }
    }
}

Exception::Exception(const Exception& rhs)
    : message_(rhs.message_)
{
}

Exception& Exception::operator=(const Exception& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }

    message_ = rhs.message_;
    return *this;
}

const char* Exception::what() const noexcept
{
    return message_.c_str();
}

const char* Exception::title() const noexcept
{
    return "Exception";
}

std::string Exception::fullMessage() const
{
    return std::string(title()).append(": ").append(what()).append(causeMessage_);
}

ConfigException::ConfigException(const std::string& message, const std::exception* cause)
    : Exception(message, cause)
{
}

const char* ConfigException::title() const noexcept
{
    return "Configuration Exception";
}

namespace
{
    std::string makeInputExceptionMessage(const std::string& detail, const std::filesystem::path& file, const std::string& token)
    {
        std::string message;

        if (!file.empty())
            message.append("while reading file \"").append(file.string()).append("\": ");

        message.append(detail);

        if (!token.empty())
            message.append(" (\"").append(token).append("\")");

        return message;
    }
}

InputException::InputException(const std::string& message, const std::filesystem::path& file,
    const std::string& token, const std::exception* cause)
    : Exception(makeInputExceptionMessage(message, file, token), cause)
{
}

const char* InputException::title() const noexcept
{
    return "Input Exception";
}

namespace
{
    std::string makeOutputExceptionMessage(const std::string& detail, const std::filesystem::path& file)
    {
        std::string message;

        if (!file.empty())
            message.append("while writing file \"").append(file.string()).append("\": ");

        message.append(detail);

        return message;
    }
}

EmbedException::EmbedException(const std::string& message)
    : Exception(message)
{
}

const char* EmbedException::title() const noexcept
{
    return "Embed Exception";
}

OutputException::OutputException(const std::string& message, const std::filesystem::path& file,
    const std::exception* cause)
    : Exception(makeOutputExceptionMessage(message, file), cause)
{
}

const char* OutputException::title() const noexcept
{
    return "Output Exception";
}
