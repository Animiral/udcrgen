#include "wudcrgen.h"
#include "graph.h"
#include "svg.h"
#include "config.h"
#include <iostream>
#include <fstream>
#include <iomanip>

extern void test_all();
void write_output(const DiskGraph& udcrg, std::ostream& stream);

int main(int argc, const char* argv[])
{
	std::cout << "Run tests...\n";
	test_all();
	std::cout << "Tests Done.\n";

	Configuration configuration;

	try {
		configuration.readArgv(argc, argv);
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to read configuration from command line: " << e.what() << "\n";
		return 1;
	}

	configuration.dump(std::cout);

	// DEBUG: in the current stage of development, config is partially hardcoded.
	if (configuration.inputFile.empty())
	{
		configuration.inputFile = "example.txt";
	}

	Caterpillar input;

	try {
		std::cout << "Process input file " << configuration.inputFile << "...\n";
		std::ifstream stream{ configuration.inputFile };
		input = Caterpillar::fromText(stream);
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to read input file \"" << configuration.inputFile << "\": " << e.what() << "\n";
		return 1;
	}

	try {
		// DEBUG: in the current stage of development, we always produce two outputs (hardcoded).
		{
			// run strong embedding algorithm (not actually strong yet)
			auto output = udcrgen(input, configuration.gap);

			// write output to predefined file
			constexpr auto outfilename = "udcrgen.txt";
			constexpr auto outsvgname = "udcrgen.svg";
			std::ofstream stream{ outfilename };
			write_output(output, stream);
			write_svg(output, outsvgname);
		}

		{
			// run weak embedding algorithm
			auto output = wudcrgen(input);

			// write output to predefined file
			constexpr auto outfilename = "wudcrgen.txt";
			constexpr auto outsvgname = "wudcrgen.svg";
			std::ofstream stream{ outfilename };
			write_output(output, stream);
			write_svg(output, outsvgname);
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to determine graph embedding: " << e.what() << "\n";
		return 1;
	}

	std::cout << "All Done.\n";
	return 0;
}

/**
 * A simple text dump of the output graph that does not yet look very pretty.
 */
void write_output(const DiskGraph& udcrg, std::ostream& stream)
{
	stream << std::setprecision(2);

	std::vector<Disk> disks;
	disks.insert(disks.end(), udcrg.spines().begin(), udcrg.spines().end());
	disks.insert(disks.end(), udcrg.branches().begin(), udcrg.branches().end());
	disks.insert(disks.end(), udcrg.leaves().begin(), udcrg.leaves().end());

	for (const auto& v : disks) {
		if (v.failure) {
			stream << "FAILED to place disk " << v.id << " -> " << v.parent << ".\n";
		}
		else {
			stream << v.id << " -> " << v.parent << "  (" << v.x << ", " << v.y << ")\n";
		}
	}
}
