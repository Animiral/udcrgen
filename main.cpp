#include "embed.h"
#include "graph.h"
#include "ipe.h"
#include "svg.h"
#include "config.h"
#include <iostream>
#include <fstream>
#include <iomanip>

void write_output(const DiskGraph& udcrg, std::ostream& stream);

int main(int argc, const char* argv[])
{
	Configuration configuration;

	try {
		configuration.readArgv(argc, argv);
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to read configuration from command line: " << e.what() << "\n";
		return 1;
	}

	configuration.dump(std::cout);

	DiskGraph* graph = nullptr;

	try {
		std::cout << "Process input file " << configuration.inputFile << "...\n";
		std::ifstream stream{ configuration.inputFile };

		switch (configuration.inputFormat) {

		case Configuration::InputFormat::DEGREES:
			graph = new DiskGraph{ DiskGraph::fromCaterpillar(Caterpillar::fromText(stream)) };
			break;

		default:
		case Configuration::InputFormat::EDGELIST:
			EdgeList edges = edges_from_text(stream);
			auto result = classify(edges);
			graph = new DiskGraph{ result.first };
			break;

		}

		stream.close();
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to read input file \"" << configuration.inputFile << "\": " << e.what() << "\n";
		return 1;
	}

	try {
		Embedder* embedder = nullptr;

		switch (configuration.algorithm) {

		default:
		case Configuration::Algorithm::KLEMZ_NOELLENBURG_PRUTKIN:
		{
			auto properEmbedder = new ProperEmbedder();
			properEmbedder->setGap(configuration.gap);
			embedder = properEmbedder;
		}
			break;

		case Configuration::Algorithm::CLEVE:
			embedder = new WeakEmbedder(graph->spines().size());
			break;

		}

		embed(*graph, *embedder);
		delete embedder;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to determine graph embedding: " << e.what() << "\n";
		return 1;
	}

	try {
		std::ofstream stream{ configuration.outputFile };

		switch (configuration.outputFormat) {

		case Configuration::OutputFormat::DUMP:
		{
			write_output(*graph, stream);
		}
			break;

		case Configuration::OutputFormat::SVG:
		{
			Svg svg{ *graph, stream };
			svg.write();
		}
			break;

		case Configuration::OutputFormat::IPE:
		{
			Ipe ipe{ *graph, stream };
			ipe.write();
		}
			break;

		}

		stream.close();
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to write output file \"" << configuration.outputFile << "\": " << e.what() << "\n";
		return 1;
	}

	delete graph;

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
