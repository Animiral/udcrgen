#include "config.h"
#include "utility/util.h"
#include "utility/exception.h"
#include <string>
#include <iomanip>
#include <functional>
#include <filesystem>
#include <stdexcept>
#include <cassert>

using namespace std::string_literals;

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
            throw ConfigException("Command line unexpectedly short."s);

        argc--;
        return *++argv;
    }

    /**
     * Describes the different kinds of argument values that this parser recognizes.
     */
    enum class Token
    {
        LITERAL,
        ALGORITHM,
        INPUT_FILE, OUTPUT_FILE, STATS_FILE, ARCHIVE_YES, ARCHIVE_NO,
        INPUT_FORMAT, OUTPUT_FORMAT,
        EMBED_ORDER,
        
        GAP,

        SPINE_MIN, SPINE_MAX, BATCH_SIZE,

        OPT_END
    };

    /**
     * Determine the token type of the current argument in front.
     */
    Token what() const
    {
        assert(!end());
        const auto opt = argv[0];

        if ("-a"s == opt || "--algorithm"s == opt)     return Token::ALGORITHM;
        if ("-i"s == opt || "--input-file"s == opt)    return Token::INPUT_FILE;
        if ("-o"s == opt || "--output-file"s == opt)   return Token::OUTPUT_FILE;
        if ("-s"s == opt || "--stats-file"s == opt)    return Token::STATS_FILE;
        if ("--archive-yes"s == opt)                   return Token::ARCHIVE_YES;
        if ("--archive-no"s == opt)                    return Token::ARCHIVE_NO;
        if ("-j"s == opt || "--input-format"s == opt)  return Token::INPUT_FORMAT;
        if ("-f"s == opt || "--output-format"s == opt) return Token::OUTPUT_FORMAT;
        if ("-e"s == opt || "--embed-order"s == opt)   return Token::EMBED_ORDER;

        if ("-g"s == opt || "--gap"s == opt)           return Token::GAP;

        if ("--spine-min"s == opt)                     return Token::SPINE_MIN;
        if ("--spine-max"s == opt)                     return Token::SPINE_MAX;
        if ("--batch-size"s == opt)                    return Token::BATCH_SIZE;

        if ("--"s == opt)                              return Token::OPT_END;

        return Token::LITERAL;
    }

    /**
     * Interpret the next argument value as an algorithm specification.
     *
     * @return: the argument parsed into an Algorithm
     * @throw ConfigException: if the argument cannot be interpreted
     */
    Configuration::Algorithm algorithm()
    {
        using namespace std::string_literals;

        const auto opt = next();

        if ("knp"s == opt || "strict"s == opt || "strong"s == opt) return Configuration::Algorithm::KLEMZ_NOELLENBURG_PRUTKIN;
        if ("cleve"s == opt || "weak"s == opt)                     return Configuration::Algorithm::CLEVE;
        if ("dp"s == opt || "dynamic-program"s == opt)             return Configuration::Algorithm::DYNAMIC_PROGRAM;
        if ("benchmark"s == opt)                                   return Configuration::Algorithm::BENCHMARK;

        throw ConfigException("Unknown algorithm: "s += opt);
    }

    /**
     * Interpret the next argument value as an input format specification.
     *
     * @return: the argument parsed into an InputFormat
     * @throw ConfigException: if the argument cannot be interpreted
     */
    Configuration::InputFormat inputFormat()
    {
        using namespace std::string_literals;

        const auto opt = next();

        if ("degrees"s == opt)  return Configuration::InputFormat::DEGREES;
        if ("edgelist"s == opt) return Configuration::InputFormat::EDGELIST;

        throw ConfigException("Unknown input format: "s += opt);
    }

    /**
     * Interpret the next argument value as an output format specification.
     *
     * @return: the argument parsed into an OutputFormat
     * @throw ConfigException: if the argument cannot be interpreted
     */
    Configuration::OutputFormat outputFormat()
    {
        using namespace std::string_literals;

        const auto opt = next();

        if ("svg"s == opt)   return Configuration::OutputFormat::SVG;
        if ("ipe"s == opt)   return Configuration::OutputFormat::IPE;
        if ("dump"s == opt)  return Configuration::OutputFormat::DUMP;

        throw ConfigException("Unknown output format: "s += opt);
    }

    /**
     * Interpret the next argument value as an embed order specification.
     *
     * @return: the argument parsed into an EmbedOrder
     * @throw ConfigException: if the argument cannot be interpreted
     */
    Configuration::EmbedOrder embedOrder()
    {
        using namespace std::string_literals;

        const auto opt = next();

        if ("dfs"s == opt || "depth-first"s == opt)    return Configuration::EmbedOrder::DEPTH_FIRST;
        if ("bfs"s == opt || "breadth-first"s == opt)  return Configuration::EmbedOrder::BREADTH_FIRST;

        throw ConfigException("Unknown embed order: "s += opt);
    }

    /**
     * Interpret the next argument value as an integer value.
     *
     * @param minValue: minimum value of the argument
     * @return: the argument parsed into an integer
     * @throw ConfigException: if the argument is smaller than the minValue
     */
    int intArg(int minValue = 1)
    {
        const char* intStr = next();
        int value = 0;

        try {
            value = std::stoi(intStr);
        }
        catch (const std::exception& e) {
            throw ConfigException("Failed to parse integer: "s += intStr, &e);
        }

        if (value < minValue)
            throw ConfigException(format("Integer argument value too small: {} (< {})", value, minValue));

        return value;
    }

    /**
     * Interpret the next argument value as a floating-point value.
     *
     * @param minValue: minimum value of the argument
     * @param maxValue: maximum value of the argument
     * @return: the argument parsed into a floating-point value
     * @throw ConfigException: if the argument violates minValue or maxValue
     */
    float floatArg(float minValue = std::numeric_limits<float>::lowest(),
        float maxValue = std::numeric_limits<float>::max())
    {
        const char* floatStr = next();
        float value = 0;

        try {
            value = std::stof(floatStr);
        }
        catch (const std::exception& e) {
            throw ConfigException("Failed to parse floating-point number: "s += floatStr, &e);
        }

        if (value < minValue)
            throw ConfigException(format("Floating-point argument value too small: {} (< {})", value, minValue));

        if (value > maxValue)
            throw ConfigException(format("Floating-point argument value too large: {} (> {})", value, maxValue));

        return value;
    }

    /**
     * Interpret the next argument value as a filesystem path.
     *
     * @return: the argument parsed into a filesystem path
     * @throw ConfigException: if the argument is missing
     */
    std::filesystem::path pathArg()
    {
        const char* pathStr = next();

        try {
            return { pathStr };
        }
        catch (const std::exception& e) {
            throw ConfigException("Invalid path: "s += pathStr, &e);
        }
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
        case Parser::Token::STATS_FILE:      statsFile = parser.pathArg(); break;
        case Parser::Token::ARCHIVE_YES:     archiveYes = parser.pathArg(); break;
        case Parser::Token::ARCHIVE_NO:      archiveNo = parser.pathArg(); break;
        case Parser::Token::INPUT_FORMAT:    inputFormat = parser.inputFormat(); break;
        case Parser::Token::OUTPUT_FORMAT:   outputFormat = parser.outputFormat(); break;
        case Parser::Token::EMBED_ORDER:     embedOrder = parser.embedOrder(); break;

        case Parser::Token::GAP:             gap = parser.floatArg(0.f, 2.f); break;

        case Parser::Token::SPINE_MIN:       spineMin = parser.intArg(); break;
        case Parser::Token::SPINE_MAX:       spineMax = parser.intArg(); break;
        case Parser::Token::BATCH_SIZE:      batchSize = parser.intArg(); break;

        case Parser::Token::OPT_END:
            //inputFiles.insert(inputFiles.end(), &parser.argv[1], &parser.argv[parser.argc]);
            parser.argc = 1;
            break;
        default: assert(0);

        }

        token = parser.next();
    }

    // autocomplete non-defaults
    if (outputFile.empty() && !inputFile.empty()) { // infer output file name from input file name
        const char* ext = nullptr;

        switch (outputFormat)
        {
        case OutputFormat::SVG:
            ext = ".html";
            break;
        case OutputFormat::IPE:
            ext = ".ipe";
            break;
        default:
        case OutputFormat::DUMP:
            ext = ".dump.txt";
            break;

        }

        outputFile = inputFile;
        outputFile.replace_extension(ext);
    }
}

void Configuration::validate() const
{
    if (Algorithm::BENCHMARK == algorithm && !inputFile.empty())
        throw ConfigException("Benchmark does not use an input file.");

    if (Algorithm::BENCHMARK != algorithm && inputFile.empty())
        throw ConfigException("Please specify an input file.");

    // TODO: only SVG format in BENCHMARK mode

    if (spineMin >= spineMax)
        throw ConfigException(format("spine-min must be smaller than spine-max. ({} >= {})", spineMin, spineMax));
}

void Configuration::dump(std::ostream& stream) const
{
    stream << "Configuration:\n\n";

    stream << "(Running from: " << std::filesystem::current_path() << ")\n";

    stream << "\tAlgorithm: " << algorithmString(algorithm) << "\n";

    stream << "\tInput File: " << inputFile << " (";
    switch (inputFormat) {
    case InputFormat::DEGREES: stream << "degrees"; break;
    case InputFormat::EDGELIST: stream << "edgelist"; break;
    }
    stream << ")\n";

    stream << "\tOutput File: " << outputFile << " (";
    switch (outputFormat) {
    case OutputFormat::SVG: stream << "svg"; break;
    case OutputFormat::IPE: stream << "ipe"; break;
    case OutputFormat::DUMP: stream << "dump"; break;
    }
    stream << ")\n";
    stream << "\tStats File: " << statsFile << "\n";
    stream << "\tArchive Directory (yes-instances): " << archiveYes << "\n";
    stream << "\tArchive Directory (no-instances): " << archiveNo << "\n\n";

    stream << "\tEmbed Order: ";
    switch (embedOrder) {
    case EmbedOrder::DEPTH_FIRST: stream << "depth-first"; break;
    case EmbedOrder::BREADTH_FIRST: stream << "breadth-first"; break;
    }
    stream << "\n\n";

    stream << "\tGap: " << std::setprecision(3) << gap << "\n\n";

    stream << "\t(Benchmark) minimum spine length: " << spineMin << "\n";
    stream << "\t(Benchmark) maximum spine length: " << spineMax << "\n";
    stream << "\t(Benchmark) batch size: " << batchSize << "\n\n";

    if (stream.bad())
        throw ConfigException("Bad stream while describing configuration.");
}

const char* Configuration::algorithmString(Algorithm algorithm) noexcept
{
    switch (algorithm) {
    case Algorithm::KLEMZ_NOELLENBURG_PRUTKIN: return "knp";
    case Algorithm::CLEVE: return "cleve";
    case Algorithm::DYNAMIC_PROGRAM: return "dynamic-program";
    case Algorithm::BENCHMARK: return "benchmark";
    default: assert(0); return "?";
    }
}
