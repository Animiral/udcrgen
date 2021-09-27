#include "config.h"
#include <string>
#include <iomanip>
#include <functional>
#include <stdexcept>
#include <cassert>
#include "utility/util.h"

/**
 * The Parser object holds the state of the options parser in progress.
 * It offers functions to tokenize command arguments and extract argument values.
 */
struct Parser
{
    int argc; //!< number of remaining unparsed arguments
    const char** argv; //!< remaining unparsed arguments data

    /**
     * Determine whether the parser has reached the end of the command line.
     */
    bool end() const
    {
        return argc <= 0;
    }

    /**
     * Advance to the next command line argument.
     *
     * @return: the pointer to the next argument in front
     */
    const char* next()
    {
        if (end())
            throw std::out_of_range("Command line unexpectedly short.");

        argc--;
        return *++argv;
    }

    /**
     * Describes the different kinds of argument values that this parser recognizes.
     */
    enum class Token
    {
        LITERAL,
        ALGORITHM, INPUT_FILE, OUTPUT_FILE, INPUT_FORMAT, OUTPUT_FORMAT, GAP,
        OPT_END
    };

    /**
     * Determine the token type of the current argument in front.
     */
    Token what() const
    {
        using namespace std::string_literals;

        assert(!end());
        const auto opt = argv[0];

        if ("-a"s == opt || "--algorithm"s == opt)     return Token::ALGORITHM;
        if ("-i"s == opt || "--input-file"s == opt)    return Token::INPUT_FILE;
        if ("-o"s == opt || "--output-file"s == opt)   return Token::OUTPUT_FILE;
        if ("-j"s == opt || "--input-format"s == opt)  return Token::INPUT_FORMAT;
        if ("-f"s == opt || "--output-format"s == opt) return Token::OUTPUT_FORMAT;
        if ("-g"s == opt || "--gap"s == opt)           return Token::GAP;
        if ("--"s == opt)                              return Token::OPT_END;

        return Token::LITERAL;
    }

    /**
     * Interpret the next argument value as an algorithm specification.
     *
     * @return: the argument parsed into an Algorithm
     * @throw std::out_of_range: if the argument cannot be interpreted
     */
    Configuration::Algorithm algorithm()
    {
        using namespace std::string_literals;

        const auto opt = next();

        if ("knp"s == opt || "strict"s == opt || "strong"s == opt) return Configuration::Algorithm::KLEMZ_NOELLENBURG_PRUTKIN;
        if ("cleve"s == opt || "weak"s == opt) return Configuration::Algorithm::CLEVE;

        throw std::out_of_range("Unknown algorithm: "s + opt);
    }

    /**
     * Interpret the next argument value as an input format specification.
     *
     * @return: the argument parsed into an InputFormat
     * @throw std::out_of_range: if the argument cannot be interpreted
     */
    Configuration::InputFormat inputFormat()
    {
        using namespace std::string_literals;

        const auto opt = next();

        if ("degrees"s == opt)  return Configuration::InputFormat::DEGREES;
        if ("edgelist"s == opt) return Configuration::InputFormat::EDGELIST;

        throw std::out_of_range("Unknown input format: "s + opt);
    }

    /**
     * Interpret the next argument value as an output format specification.
     *
     * @return: the argument parsed into an OutputFormat
     * @throw std::out_of_range: if the argument cannot be interpreted
     */
    Configuration::OutputFormat outputFormat()
    {
        using namespace std::string_literals;

        const auto opt = next();

        if ("svg"s == opt)   return Configuration::OutputFormat::SVG;
        if ("ipe"s == opt)   return Configuration::OutputFormat::IPE;
        if ("dump"s == opt)  return Configuration::OutputFormat::DUMP;

        throw std::out_of_range("Unknown output format: "s + opt);
    }

    /**
     * Interpret the next argument value as an integer value.
     *
     * @param minValue: minimum value of the argument
     * @return: the argument parsed into an integer
     * @throw std::out_of_range: if the argument is smaller than the minValue
     */
    int intArg(int minValue = 1)
    {
        const int value = std::stoi(next());

        if (value < minValue)
            throw std::out_of_range(format("Integer argument value too small: {} (< {})", value, minValue));

        return value;
    }

    /**
     * Interpret the next argument value as a floating-point value.
     *
     * @param minValue: minimum value of the argument
     * @param maxValue: maximum value of the argument
     * @return: the argument parsed into a floating-point value
     * @throw std::out_of_range: if the argument violates minValue or maxValue
     */
    float floatArg(float minValue = std::numeric_limits<float>::lowest(),
        float maxValue = std::numeric_limits<float>::max())
    {
        const float value = std::stof(next());

        if (value < minValue)
            throw std::out_of_range(format("Floating-point argument value too small: {} (< {})", value, minValue));

        if (value > maxValue)
            throw std::out_of_range(format("Floating-point argument value too large: {} (> {})", value, maxValue));

        return value;
    }

    /**
     * Interpret the next argument value as a filesystem path.
     *
     * @return: the argument parsed into a filesystem path
     * @throw std::out_of_range: if the argument is missing
     */
    path pathArg()
    {
        return { next() };
    }
};

void Configuration::readArgv(int argc, const char* argv[])
{
    Parser parser{ argc, argv };
    const char* token = parser.next();

    while (!parser.end()) {
        switch (parser.what()) {

        case Parser::Token::LITERAL:         inputFile = token; break;
        case Parser::Token::ALGORITHM:       algorithm = parser.algorithm(); break;
        case Parser::Token::INPUT_FILE:      inputFile = parser.pathArg(); break;
        case Parser::Token::OUTPUT_FILE:     outputFile = parser.pathArg(); break;
        case Parser::Token::INPUT_FORMAT:    inputFormat = parser.inputFormat(); break;
        case Parser::Token::OUTPUT_FORMAT:   outputFormat = parser.outputFormat(); break;
        case Parser::Token::GAP:             gap = parser.floatArg(0.f, 2.f); break;
        case Parser::Token::OPT_END:
            //inputFiles.insert(inputFiles.end(), &parser.argv[1], &parser.argv[parser.argc]);
            parser.argc = 1;
            break;
        default: assert(0);

        }

        token = parser.next();
    }

    // autocomplete non-defaults
    if (outputFile.empty()) {
        const char* ext = nullptr;

        switch (outputFormat)
        {
        case OutputFormat::SVG:
            ext = ".svg";
            break;
        case OutputFormat::IPE:
            ext = ".ipe";
            break;
        default:
        case OutputFormat::DUMP:
            ext = ".dump.txt";
            break;

        }

        outputFile = inputFile + ext;
    }
}

void Configuration::dump(std::ostream& stream) const
{
    stream << "Configuration:\n\n";

    stream << "\tAlgorithm: ";
    switch (algorithm) {
    case Algorithm::KLEMZ_NOELLENBURG_PRUTKIN: stream << "knp"; break;
    case Algorithm::CLEVE: stream << "cleve"; break;
    }
    stream << "\n";

    stream << "\tInput File: " << inputFile << " (";
    switch (inputFormat) {
    case InputFormat::DEGREES: stream << "degrees"; break;
    case InputFormat::EDGELIST: stream << "edgelist"; break;
    }
    stream << ")\n";

    stream << "\tOutput File: " << outputFile << " (";
    switch (outputFormat) {
    case OutputFormat::SVG: stream << "svg"; break;
    case OutputFormat::DUMP: stream << "dump"; break;
    }
    stream << ")\n";

    stream << "\tGap: " << std::setprecision(3) << gap << "\n\n";
}
