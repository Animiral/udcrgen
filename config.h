/**
 * Contains infrastructure for reading program configuration from program arguments and sources.
 */
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
     * Enumeration of available generators to run as the main mode of the program.
     */
    enum class Algorithm { KLEMZ_NOELLENBURG_PRUTKIN, CLEVE };

    /**
     * Enumeration of available file formats for input files.
     */
    enum class InputFormat { DEGREES, EDGELIST };

    /**
     * Enumeration of available file formats for output files.
     */
    enum class OutputFormat { SVG, DUMP };

    Algorithm algorithm = Algorithm::KLEMZ_NOELLENBURG_PRUTKIN;
    path inputFile; //!< only one supported at this time
    path outputFile; //!< only one supported at this time
    InputFormat inputFormat = InputFormat::DEGREES;
    OutputFormat outputFormat = OutputFormat::SVG;
    float gap = .1f; //!< size of gap between non-contact disks in strong UDCRs

    /**
     * Write the parsed information in the configuration to the given stream.
     */
    void dump(std::ostream& stream) const;

};
