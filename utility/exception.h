/**
 * Defines the base exception for error handling in udcrgen.
 *
 * More specific derived exceptions are defined in the relevant modules themselves.
 */
#pragma once

#include <stdexcept>
#include <concepts>
#include <string>
#include "util.h"

/**
 * @brief Base exception class for all errors that are caused by issues in the
 * domain of this program.
 *
 * It supports stacking exceptions as causes and uniform pretty printing.
 */
class Exception : public std::exception
{

public:

    Exception();
    explicit Exception(const std::string& message, const std::exception* cause = nullptr);
    Exception(const Exception& rhs);
    Exception& operator=(const Exception& rhs);

    [[nodiscard]] virtual const char* what() const noexcept override;

    /**
     * An exception type-specific title that informs the user about the general
     * nature of the problem. To be formatted before the colon in error message.
     */
    virtual const char* title() const noexcept;

    /**
     * Return the title and message of this and all @c cause exceptions formatted
     * into a uniform user-readable format.
     */
    std::string fullMessage() const;

private:

    std::string message_;
    std::string causeMessage_;

};
