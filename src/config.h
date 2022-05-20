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
    enum class LogLevel { ERROR, INFO, TRACE };

    /**
     * The log mode describes where log messages should go.
     */
    enum class LogMode { DEFAULT, STDERR, FILE, BOTH, NONE };

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

    LogLevel logLevel = LogLevel::INFO;
    LogMode logMode = LogMode::DEFAULT;
    std::filesystem::path logFile;

    /**
     * @brief Throw an exception if the configuration values are not sensible.
     */
    void validate() const;

    /**
     * Write the parsed information in the configuration to the given stream.
     */
    void dump(std::ostream& stream) const;

    /**
     * Return a human-readable representation of the algorithm enumeration value.
     *
     * This matches the expected command line value to select the algorithm to run.
     */
    static const char* algorithmString(Algorithm algorithm) noexcept;

};

int operator<=>(Configuration::LogLevel a, Configuration::LogLevel b) noexcept;
