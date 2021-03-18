#include "wudcrgen.h"
#include "graph.h"
#include "svg.h"
#include <iostream>
#include <fstream>
#include <iomanip>

extern void test_all();
void write_output(const UdcrGraph& udcrg, std::ostream& stream);

int main(int argc, const char* argv[])
{
	std::cout << "Run tests...\n";
	test_all();
	std::cout << "Tests Done.\n";

	// read input from file arg
	auto infilename = argv[1];

	if (argc < 2) {
		std::cout << "No input file. Usage: " << argv[0] << " [file]\n";
		// return 1;
		infilename = "example.txt"; // just for show
	}

	Caterpillar input;

	try {
		std::cout << "Process input file " << infilename << "...\n";
		std::ifstream stream{ infilename };
		input = Caterpillar::fromText(stream);
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to read input file \"" << argv[1] << "\": " << e.what() << "\n";
		return 1;
	}

	try {
		{
			// run strong embedding algorithm (not actually strong yet)
			auto output = udcrgen(input);

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
void write_output(const UdcrGraph& udcrg, std::ostream& stream)
{
	stream << std::setprecision(2);

	for (const auto& v : udcrg.vertices()) {
		stream << v.id << " -> " << v.parent << "  (" << v.x << ", " << v.y << ")\n";
	}
}
