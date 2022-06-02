#include "config.h"
#include "utility/util.h"
#include "utility/log.h"
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
        BENCHMARK_BFS, BENCHMARK_DFS, BENCHMARK_DYNAMIC,

        LOG_LEVEL, LOG_MODE, LOG_FILE,

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
        if ("--benchmark-bfs"s == opt)                 return Token::BENCHMARK_BFS;
        if ("--benchmark-dfs"s == opt)                 return Token::BENCHMARK_DFS;
        if ("--benchmark-dynamic"s == opt)             return Token::BENCHMARK_DYNAMIC;

        if ("-v"s == opt || "--log-level"s == opt)     return Token::LOG_LEVEL;
        if ("--log-mode"s == opt)                      return Token::LOG_MODE;
        if ("--log-file"s == opt)                      return Token::LOG_FILE;

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
        const auto opt = next();

        if ("dfs"s == opt || "depth-first"s == opt)    return Configuration::EmbedOrder::DEPTH_FIRST;
        if ("bfs"s == opt || "breadth-first"s == opt)  return Configuration::EmbedOrder::BREADTH_FIRST;

        throw ConfigException("Unknown embed order: "s += opt);
    }

    /**
     * Interpret the next argument value as a log level.
     *
     * @return: the argument parsed into a LogLevel
     * @throw ConfigException: if the argument cannot be interpreted
     */
    Configuration::LogLevel logLevel()
    {
        const auto opt = next();

        if ("silent"s == opt)  return Configuration::LogLevel::SILENT;
        if ("error"s == opt)   return Configuration::LogLevel::ERROR;
        if ("info"s == opt)    return Configuration::LogLevel::INFO;
        if ("trace"s == opt)   return Configuration::LogLevel::TRACE;

        throw ConfigException("Unknown log level: "s += opt);
    }

    /**
     * Interpret the next argument value as a log mode.
     *
     * The log mode value @c DEFAULT is reserved and will not be parsed.
     *
     * @return: the argument parsed into a LogMode
     * @throw ConfigException: if the argument cannot be interpreted
     */
    Configuration::LogMode logMode()
    {
        const auto opt = next();

        if ("stderr"s == opt)  return Configuration::LogMode::STDERR;
        if ("file"s == opt)    return Configuration::LogMode::FILE;
        if ("both"s == opt)    return Configuration::LogMode::BOTH;

        throw ConfigException("Unknown log mode: "s += opt);
    }

    /**
     * Interpret the next argument value as a boolean value.
     *
     * @return: the argument parsed into a boolean
     * @throw ConfigException: if the argument is cannot be interpreted
     */
    bool boolArg()
    {
        const char* boolStr = next();

        if ("true"s == boolStr)
            return true;
        else if ("false"s == boolStr)
            return false;
        else
            throw ConfigException("Failed to parse boolean: {}"s, boolStr);
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
            throw ConfigException("Integer argument value too small: {} (< {})", value, minValue);

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
            throw ConfigException("Floating-point argument value too small: {} (< {})", value, minValue);

        if (value > maxValue)
            throw ConfigException("Floating-point argument value too large: {} (> {})", value, maxValue);

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
    argv0 = std::filesystem::path(argv[0]).filename();

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
        case Parser::Token::BENCHMARK_BFS:     benchmarkBfs = parser.boolArg(); break;
        case Parser::Token::BENCHMARK_DFS:     benchmarkDfs = parser.boolArg(); break;
        case Parser::Token::BENCHMARK_DYNAMIC: benchmarkDynamic = parser.boolArg(); break;

        case Parser::Token::LOG_LEVEL:       logLevel = parser.logLevel(); break;
        case Parser::Token::LOG_MODE:        logMode = parser.logMode(); break;
        case Parser::Token::LOG_FILE:        logFile = parser.pathArg(); break;

        case Parser::Token::OPT_END:
            //inputFiles.insert(inputFiles.end(), &parser.argv[1], &parser.argv[parser.argc]);
            parser.argc = 1;
            break;
        default: assert(0);

        }

        token = parser.next();
    }
}

void Configuration::validate() const
{
    if (Algorithm::BENCHMARK == algorithm && !inputFile.empty())
        throw ConfigException("Benchmark does not use an input file.");

    if (Algorithm::BENCHMARK == algorithm && OutputFormat::SVG != outputFormat)
        throw ConfigException("Benchmark supports only SVG as output format.");

    if (Algorithm::BENCHMARK != algorithm && inputFile.empty())
        throw ConfigException("Please specify an input file.");

    if (spineMin >= spineMax)
        throw ConfigException("spine-min must be smaller than spine-max. ({} >= {})", spineMin, spineMax);

    if (LogLevel::SILENT == logLevel && (LogMode::DEFAULT != logMode || !logFile.empty()))
        throw ConfigException("No other log options may be combined with silent mode.");

    if ((LogMode::STDERR == logMode) && !logFile.empty())
        throw ConfigException("Please specify a log mode that includes file logging if you specify a log file.");
}

void Configuration::finalize()
{
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

    if (LogMode::DEFAULT == logMode) {
        if (logFile.empty())
            logMode = LogMode::STDERR;
        else
            logMode = LogMode::FILE;
    }
    else {
        if ((LogMode::FILE == logMode || LogMode::BOTH == logMode) && logFile.empty()) {
            logFile = argv0;
            logFile.append(".log");
        }
    }
}

void Configuration::dump() const
{
    theLog->writeRaw(LogLevel::INFO, "\n=== Configuration ===\n\n");

    theLog->writeRaw(LogLevel::INFO, "Working directory: {}\n", std::filesystem::current_path());
    theLog->writeRaw(LogLevel::INFO, "Algorithm: {}\n\n", algorithmString(algorithm));

    theLog->writeRaw(LogLevel::INFO, "= Files =\n");
    if (Algorithm::BENCHMARK == algorithm) {
        theLog->writeRaw(LogLevel::INFO, "\tArchive Directory (yes-instances): {}\n", archiveYes);
        theLog->writeRaw(LogLevel::INFO, "\tArchive Directory (no-instances): {}\n", archiveNo);
    }
    else {
        theLog->writeRaw(LogLevel::INFO, "\tInput File: {} ({})\n", inputFile, inputFormatString(inputFormat));
        theLog->writeRaw(LogLevel::INFO, "\tOutput File: {} ({})\n", outputFile, outputFormatString(outputFormat));
    }
    theLog->writeRaw(LogLevel::INFO, "\tStats File: {}\n\n", statsFile);

    if (Algorithm::DYNAMIC_PROGRAM != algorithm) {
        theLog->writeRaw(LogLevel::INFO, "= Algorithmic Parameters =\n");
    }

    if (Algorithm::BENCHMARK == algorithm) {
        theLog->writeRaw(LogLevel::INFO, "\tMinimum spine length: {}\n", spineMin);
        theLog->writeRaw(LogLevel::INFO, "\tMaximum spine length: {}\n", spineMax);
        if (batchSize > 0)
            theLog->writeRaw(LogLevel::INFO, "\tBatch size: {}\n", batchSize);
        theLog->writeRaw(LogLevel::INFO, "\tBenchmark heuristic with BFS order: {}\n", benchmarkBfs);
        theLog->writeRaw(LogLevel::INFO, "\tBenchmark heuristic with DFS order: {}\n", benchmarkBfs);
        theLog->writeRaw(LogLevel::INFO, "\tBenchmark dynamic program: {}\n", benchmarkDynamic);
    }
    if (Algorithm::KLEMZ_NOELLENBURG_PRUTKIN == algorithm) {
        theLog->writeRaw(LogLevel::INFO, "\tGap: {}{}\n\n", std::setprecision(3), gap);
    }
    if (Algorithm::CLEVE == algorithm || Algorithm::BENCHMARK == algorithm) {
        theLog->writeRaw(LogLevel::INFO, "\tEmbed Order: {}\n\n", embedOrderString(embedOrder));
    }

    theLog->writeRaw(LogLevel::INFO, "= Logging =\n");
    theLog->writeRaw(LogLevel::INFO, "\tLog level: {}\n", logLevelString(logLevel));

    if (LogLevel::SILENT != logLevel) {
        theLog->writeRaw(LogLevel::INFO, "\tLog mode: {}\n", logModeString(logMode));
        if (LogMode::FILE == logMode || LogMode::BOTH == logMode) {
            theLog->writeRaw(LogLevel::INFO, "\tLog file: {}\n", logFile);
        }
    }
    theLog->writeRaw(LogLevel::INFO, "\n");

    theLog->writeRaw(LogLevel::INFO, "=== Configuration ===\n\n");
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

const char* Configuration::inputFormatString(InputFormat inputFormat) noexcept
{
    switch (inputFormat) {
    case InputFormat::DEGREES: return "degrees";
    case InputFormat::EDGELIST: return "edgelist";
    default: assert(0); return "?";
    }
}

const char* Configuration::outputFormatString(OutputFormat outputFormat) noexcept
{
    switch (outputFormat) {
    case OutputFormat::SVG: return "svg";
    case OutputFormat::IPE: return "ipe";
    case OutputFormat::DUMP: return "dump";
    default: assert(0); return "?";
    }
}

const char* Configuration::embedOrderString(EmbedOrder embedOrder) noexcept
{
    switch (embedOrder) {
    case EmbedOrder::DEPTH_FIRST: return "depth-first";
    case EmbedOrder::BREADTH_FIRST: return "breadth-first";
    default: assert(0); return "?";
    }
}

const char* Configuration::logLevelString(LogLevel logLevel) noexcept
{
    switch (logLevel) {
    case LogLevel::SILENT: return "silent";
    case LogLevel::ERROR: return "error";
    case LogLevel::INFO: return "info";
    case LogLevel::TRACE: return "trace";
    default: assert(0); return "?";
    }
}

const char* Configuration::logModeString(LogMode logMode) noexcept
{
    switch (logMode) {
    case LogMode::DEFAULT: return "(default)";
    case LogMode::STDERR: return "stderr";
    case LogMode::FILE: return "file";
    case LogMode::BOTH: return "both";
    default: assert(0); return "?";
    }
}

int operator<=>(Configuration::LogLevel a, Configuration::LogLevel b) noexcept
{
    return static_cast<int>(b) - static_cast<int>(a);
}
