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
