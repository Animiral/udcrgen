#include "embed.h"
#include "heuristic.h"
#include "dynamic.h"
#include "enumerate.h"
#include "utility/graph.h"
#include "output/ipe.h"
#include "output/svg.h"
#include "output/csv.h"
#include "config.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cassert>

namespace
{
	Configuration configuration;

	// These functions implement the phases of the program according to the configuration:
	// configure -> input -> processing -> output.
	//
	// The exception is BENCHMARK mode, in which the input is procedurally generated
	// and the input file and input graph are ignored/empty. In that case, the output
	// is a stats file in CSV format.
	void build_configuration(int argc, const char* argv[]);
	DiskGraph read_input_graph();
	void run_algorithm(DiskGraph& graph);
	void run_benchmark();
	void write_output_graph(const DiskGraph& graph);
	void write_output_stats(const std::vector<Stat>& stats);

	// basic text dump for debugging
	void write_output_graph_stream(const DiskGraph& udcrg, std::ostream& stream);
}

int main(int argc, const char* argv[])
{
	try
	{
		build_configuration(argc, argv);

		if (Configuration::Algorithm::BENCHMARK == configuration.algorithm) {
			run_benchmark();
		}
		else {
			DiskGraph graph = read_input_graph();
			run_algorithm(graph);
			write_output_graph(graph);
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}

	std::cout << "All Done.\n";
	return 0;
}

namespace
{

void build_configuration(int argc, const char* argv[])
{
	try {
		configuration.readArgv(argc, argv);
	}
	catch (const std::exception& e) {
		using namespace std::string_literals;
		throw std::exception(("Failed to read configuration from command line: "s + e.what() + "\n"s).c_str());
	}

	configuration.dump(std::cout);
}

DiskGraph read_input_graph()
{
	try {
		if (configuration.inputFile.empty())
			return DiskGraph(1, 0, 0);

		std::cout << "Process input file " << configuration.inputFile << "...\n";
		std::ifstream stream{ configuration.inputFile };

		switch (configuration.inputFormat) {

		case Configuration::InputFormat::DEGREES:
		{
			DiskGraph graph = DiskGraph::fromCaterpillar(Caterpillar::fromText(stream));
			stream.close();
			return graph;
		}

		default:
		case Configuration::InputFormat::EDGELIST:
		{
			EdgeList edges = edges_from_text(stream);
			auto result = classify(edges);
			stream.close();
			return result.first;
		}

		}
	}
	catch (const std::exception& e) {
		using namespace std::string_literals;
		throw std::exception(("Failed to read input file \""s + configuration.inputFile + "\": "s + e.what() + "\n"s).c_str());
	}
}

void run_algorithm(DiskGraph& graph)
{
	try {
		switch (configuration.algorithm) {

		case Configuration::Algorithm::KLEMZ_NOELLENBURG_PRUTKIN:
		{
			ProperEmbedder embedder;
			embedder.setGap(configuration.gap);
			embed(graph, embedder, configuration.embedOrder);
		}
		break;

		case Configuration::Algorithm::CLEVE:
		{
			WeakEmbedder embedder;
			embed(graph, embedder, configuration.embedOrder);
		}
		break;

		case Configuration::Algorithm::DYNAMIC_PROGRAM:
		{
			DynamicProblemEmbedder embedder;
			embedDynamic(graph, embedder);
		}
		break;

		default: assert(0);
		break;

		}
	}
	catch (const std::exception& e) {
		using namespace std::string_literals;
		throw std::exception(("Failed to determine graph embedding: "s + e.what() + "\n"s).c_str());
	}
}

void run_benchmark()
{
	try {
		WeakEmbedder fastEmbedder;
		DynamicProblemEmbedder referenceEmbedder;
		Enumerate enumerate(fastEmbedder, referenceEmbedder, 1, 2);
		enumerate.run();
		write_output_stats(enumerate.stats());
	}
	catch (const std::exception& e) {
		using namespace std::string_literals;
		throw std::exception(("Benchmark failed: "s + e.what() + "\n"s).c_str());
	}
}

void write_output_graph(const DiskGraph& graph)
{
	try {
		if (configuration.outputFile.empty())
			return;

		std::ofstream stream{ configuration.outputFile };

		switch (configuration.outputFormat) {

		case Configuration::OutputFormat::DUMP:
		{
			write_output_graph_stream(graph, stream);
		}
		break;

		case Configuration::OutputFormat::SVG:
		{
			Svg svg(stream);
			svg.intro();
			svg.write(graph, "Embed Result");
			svg.outro();
		}
		break;

		case Configuration::OutputFormat::IPE:
		{
			Ipe ipe{ graph, stream };
			ipe.write();
		}
		break;

		}

		stream.close();
	}
	catch (const std::exception& e) {
		using namespace std::string_literals;
		throw std::exception(("Failed to write output file \""s + configuration.outputFile + "\": "s + e.what() + "\n"s).c_str());
	}
}

void write_output_stats(const std::vector<Stat>& stats)
{
	try {
		std::ofstream stream{ configuration.outputFile };
		Csv csv(stream, ',');
		csv.header();

		for (const Stat& stat : stats) {
			csv.write(stat);
		}

		stream.close();
	}
	catch (const std::exception& e) {
		using namespace std::string_literals;
		throw std::exception(("Failed to write stats file \""s + configuration.outputFile + "\": "s + e.what() + "\n"s).c_str());
	}
}

void write_output_graph_stream(const DiskGraph& udcrg, std::ostream& stream)
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

}