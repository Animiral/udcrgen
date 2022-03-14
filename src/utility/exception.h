/**
 * Defines all domain-specific exceptions for error handling in udcrgen.
 */
#pragma once

#include <exception>
#include <filesystem>
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

/**
 * @brief This class covers errors which arise during configuration.
 *
 * The cases are:
 *   - command-line arguments missing, malformed or unrecognized
 *   - validation failure of the configuration
 *   - stream error while writing configuration state
 */
class ConfigException : public Exception
{

public:

    explicit ConfigException(const std::string& message, const std::exception* cause = nullptr);
    virtual const char* title() const noexcept override;

};

/**
 * @brief This class covers errors which arise during reading input.
 *
 * The cases are:
 *   - input file cannot be opened or read
 *   - input file token cannot be parsed
 *   - input data cannot be converted to a graph
 */
class InputException : public Exception
{

public:

    /**
     * @brief Construct the Exception.
     *
     * @param message failure description
     * @param file source input file name
     * @param token problematic excerpt from input file contents
     * @param cause low-level source exception
     */
    explicit InputException(const std::string& message, const std::filesystem::path& file = {},
        const std::string& token = "", const std::exception* cause = nullptr);

    virtual const char* title() const noexcept override;

};

/**
 * @brief This class covers errors which arise when running the embedding algorithm.
 *
 * This occurs when the chosen algorithm is unsuitable for the input graph.
 */
class EmbedException : public Exception
{

public:

    explicit EmbedException(const std::string& message);
    virtual const char* title() const noexcept override;

};

/**
 * @brief This class covers errors which arise during writing output.
 *
 * This occurs when the output file cannot be opened or written to.
 */
class OutputException : public Exception
{

public:

    /**
     * @brief Construct the Exception.
     *
     * @param message failure description
     * @param file target output file name
     * @param cause low-level source exception
     */
    explicit OutputException(const std::string& message, const std::filesystem::path& file = {},
        const std::exception* cause = nullptr);

    virtual const char* title() const noexcept override;

};
