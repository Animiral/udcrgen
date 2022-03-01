/**
 * Contains infrastructure for reading program configuration from program arguments and sources.
 */
#pragma once

#include <vector>
#include <string>
#include <ostream>
//#include <filesystem>
#include <exception>

using path = std::string; // std::filesystem::path

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

    Algorithm algorithm = Algorithm::CLEVE;
    path inputFile;
    path outputFile;
    path statsFile;
    InputFormat inputFormat = InputFormat::EDGELIST;
    OutputFormat outputFormat = OutputFormat::SVG;
    EmbedOrder embedOrder = EmbedOrder::DEPTH_FIRST;
    float gap = .1f; //!< size of gap between non-contact disks in strong UDCRs

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
