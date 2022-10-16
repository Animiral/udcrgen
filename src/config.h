/**
 * Contains infrastructure for reading program configuration from program arguments and sources.
 */
#pragma once

#include <vector>
#include <string>
#include <ostream>
#include <filesystem>

/**
 * Contains all collected and parsed settings for executing a program run.
 */
class Configuration
{

public:

    /**
     * Parse configuration from program arguments.
     */
    void readArgv(int argc, const char* argv[]);

    /**
     * Main modes of the program available to run.
     */
    enum class Algorithm { KLEMZ_NOELLENBURG_PRUTKIN, CLEVE, DYNAMIC_PROGRAM, BENCHMARK };

    /**
     * Enumeration of available file formats for input files.
     */
    enum class InputFormat { DEGREES, EDGELIST };

    /**
     * Enumeration of available file formats for output files.
     */
    enum class OutputFormat { SVG, IPE, DUMP };

    /**
     * Heuristic preference for order of embedding.
     *
     * DEPTH_FIRST ... embed leaves before the next branch
     * BREADTH_FIRST ... embed all branches before leaves
     */
    enum class EmbedOrder { DEPTH_FIRST, BREADTH_FIRST };

    /**
     * Ordered set of log message levels which can be restricted.
     */
    enum class LogLevel { SILENT = 0, ERROR = 1, INFO = 2, TRACE = 3 };

    /**
     * The log mode describes where log messages should go.
     */
    enum class LogMode { DEFAULT, STDERR, FILE, BOTH };

    std::filesystem::path argv0; //!< invoked program name
    Algorithm algorithm = Algorithm::CLEVE;
    std::filesystem::path inputFile;
    std::filesystem::path outputFile;
    std::filesystem::path statsFile;
    std::filesystem::path archiveYes;
    std::filesystem::path archiveNo;
    InputFormat inputFormat = InputFormat::EDGELIST;
    OutputFormat outputFormat = OutputFormat::SVG;
    EmbedOrder embedOrder = EmbedOrder::DEPTH_FIRST;

    // keys for graphical rendering
    float gap = .1f; //!< size of gap between non-contact disks in strong UDCRs

    // benchmark-specific keys
    int spineMin = 2;
    int spineMax = 3;
    int batchSize = 0;
    bool benchmarkBfs = true;
    bool benchmarkDfs = true;
    bool benchmarkDynamic = true;

    LogLevel logLevel = LogLevel::INFO;
    LogMode logMode = LogMode::DEFAULT;
    std::filesystem::path logFile;

    /**
     * @brief Throw an exception if the configuration values are not sensible.
     */
    void validate() const;

    /**
     * @brief Complete some configuration values whose defaults depend on
     *        other parts of the configuration, such as the output file.
     */
    void finalize();

    /**
     * Log the parsed information in the configuration.
     */
    void dump() const;

    /**
     * Return a human-readable representation of the algorithm enumeration value.
     *
     * This matches the expected command line value to select the algorithm to run.
     */
    static const char* algorithmString(Algorithm algorithm) noexcept;

    /**
     * Return a human-readable representation of the input format enumeration value.
     *
     * This matches the expected command line value to select the input format.
     */
    static const char* inputFormatString(InputFormat inputFormat) noexcept;

    /**
     * Return a human-readable representation of the output format enumeration value.
     *
     * This matches the expected command line value to select the output format.
     */
    static const char* outputFormatString(OutputFormat outputFormat) noexcept;

    /**
     * Return a human-readable representation of the embed order enumeration value.
     *
     * This matches the expected command line value to select the embed order.
     */
    static const char* embedOrderString(EmbedOrder embedOrder) noexcept;

    /**
     * Return a human-readable representation of the log level enumeration value.
     *
     * This matches the expected command line value to select the log level.
     */
    static const char* logLevelString(LogLevel logLevel) noexcept;

    /**
     * Return a human-readable representation of the log mode enumeration value.
     *
     * This matches the expected command line value to select the log mode.
     */
    static const char* logModeString(LogMode logMode) noexcept;

};

bool operator<=(Configuration::LogLevel a, Configuration::LogLevel b) noexcept;
