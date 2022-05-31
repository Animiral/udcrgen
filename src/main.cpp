#include "config.h"
#include "embed.h"
#include "heuristic.h"
#include "dynamic.h"
#include "enumerate.h"
#include "utility/graph.h"
#include "utility/exception.h"
#include "utility/log.h"
#include "output/ipe.h"
#include "output/svg.h"
#include "output/csv.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <memory>
#include <cassert>
#include <cerrno>
#include <cstring>

namespace
{
	Configuration configuration;

	std::unique_ptr<FileLog> fileLog; // optional: log to file
	std::unique_ptr<StreamLog> streamLog; // optional: log to stderr
	std::unique_ptr<DuplicateLog> bothLog; // optional: log to file and stderr

	// These functions implement the phases of the program according to the configuration:
	// configure -> input -> processing -> output.
	//
	// The exception is BENCHMARK mode, in which the input is procedurally generated
	// and the input file and input graph are ignored/empty. In that case, the output
	// is a stats file in CSV format.
	void build_configuration(int argc, const char* argv[]);
	void setup_log(); // init log and replay memory of stage 1 log
	DiskGraph read_input_graph(); // single mode: read from specified file
	void run_algorithm(DiskGraph& graph); // run single mode on graph (except benchmark)
	void run_benchmark();
	void write_output_graph(const DiskGraph& graph); // single mode: write to output file

	// basic text dump for debugging
	void write_output_graph_stream(const DiskGraph& graph, std::ostream& stream);
}

int main(int argc, const char* argv[])
{
	try
	{
		trace("{} started with {} args, parse configuration...", argv[0], argc);
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
	catch (const Exception& e) {
		stage1log.shutdown(); // emergency preserve logs
		error(e.fullMessage());
		return 1;
	}
	catch (const std::exception& e) {
		stage1log.shutdown(); // emergency preserve logs
		error("Error: {}", e.what());
		return 1;
	}

	info("All Done.");
	return 0;
}

namespace
{

void build_configuration(int argc, const char* argv[])
{
	configuration.readArgv(argc, argv);
	configuration.validate();
	configuration.finalize();
	setup_log();
	configuration.dump();
}

void setup_log()
{
	switch (configuration.logMode) {
	default:
	case Configuration::LogMode::DEFAULT:
	case Configuration::LogMode::STDERR:
		streamLog.reset(new StreamLog(std::clog));
		theLog = streamLog.get();
		break;

	case Configuration::LogMode::FILE:
		fileLog.reset(new FileLog(configuration.logFile));
		theLog = fileLog.get();
		break;

	case Configuration::LogMode::BOTH:
		streamLog.reset(new StreamLog(std::clog));
		fileLog.reset(new FileLog(configuration.logFile));
		bothLog.reset(new DuplicateLog(*streamLog, *fileLog));
		theLog = bothLog.get();
		break;

	}

	theLog->setLevel(configuration.logLevel);
	stage1log.replay(*theLog);
}

DiskGraph read_input_graph()
{
	assert(!configuration.inputFile.empty());

	DiskGraph graph;

	info("Process input file {}...", configuration.inputFile);
	std::ifstream stream{ configuration.inputFile };
	if (!stream.is_open())
		throw InputException(std::strerror(errno), configuration.inputFile.string());

	switch (configuration.inputFormat) {

	case Configuration::InputFormat::DEGREES:
	{
		graph = DiskGraph::fromCaterpillar(Caterpillar::fromText(stream));
	}
	break;

	default:
	case Configuration::InputFormat::EDGELIST:
	{
		EdgeList edges = edges_from_text(stream);

		if (edges.empty())
			throw InputException("Graph is empty.", configuration.inputFile.string());

		GraphClass gclass;
		std::tie(graph, gclass) = classify(edges);
	}
	break;

	}

	stream.clear();
	stream.close();

	if (stream.fail())
		throw InputException(std::strerror(errno), configuration.inputFile.string());

	return graph;
}

void run_algorithm(DiskGraph& graph)
{
	Stat stat;

	switch (configuration.algorithm) {

	case Configuration::Algorithm::KLEMZ_NOELLENBURG_PRUTKIN:
	{
		ProperEmbedder embedder;
		embedder.setGap(configuration.gap);
		stat = embed(graph, embedder, configuration.algorithm, configuration.embedOrder);
	}
	break;

	case Configuration::Algorithm::CLEVE:
	{
		WeakEmbedder embedder;
		stat = embed(graph, embedder, configuration.algorithm, configuration.embedOrder);
	}
	break;

	case Configuration::Algorithm::DYNAMIC_PROGRAM:
	{
		DynamicProblemEmbedder embedder;
		stat = embedDynamic(graph, embedder);
	}
	break;

	default: assert(0);
	break;

	}

	if (!configuration.statsFile.empty()) {
		Csv csv;
		csv.open(configuration.statsFile, std::ios::app);
		csv.write(stat);
		csv.close();
	}
}

void run_benchmark()
{
	WeakEmbedder fastEmbedder;
	DynamicProblemEmbedder referenceEmbedder;
	Enumerate enumerate(fastEmbedder, referenceEmbedder, configuration.spineMin, configuration.spineMax);
	enumerate.setEmbedOrder(configuration.embedOrder);

	Svg svg;
	bool doInstances = !configuration.outputFile.empty();

	if (doInstances) {
		svg.open(configuration.outputFile);
		svg.setBatchSize(configuration.batchSize);
		svg.intro();
		enumerate.setOutput(&svg);
	}

	Csv csv;
	bool doStats = !configuration.statsFile.empty();

	if (doStats) {
		csv.open(configuration.statsFile, std::ios::out | std::ios::trunc);
		enumerate.setCsv(&csv);
	}

	Archive archive;
	bool doArchive = !configuration.archiveYes.empty() || !configuration.archiveNo.empty();

	if (doArchive) {
		archive.setPaths(configuration.archiveYes, configuration.archiveNo);
		enumerate.setArchive(&archive);
	}

	enumerate.run();

	if (doInstances) {
		svg.outro();
		svg.close();
	}

	if (doStats) {
		csv.close();
	}
}

void write_output_graph(const DiskGraph& graph)
{
	if (configuration.outputFile.empty())
		return;

	switch (configuration.outputFormat) {

	case Configuration::OutputFormat::DUMP:
	{
		std::ofstream stream{ configuration.outputFile };
		write_output_graph_stream(graph, stream);
		stream.close();
	}
	break;

	case Configuration::OutputFormat::SVG:
	{
		Svg svg(configuration.outputFile);
		svg.intro();
		svg.write(graph, "Embed Result");
		svg.outro();
		svg.close();
	}
	break;

	case Configuration::OutputFormat::IPE:
	{
		std::ofstream stream{ configuration.outputFile };
		Ipe ipe{ graph, stream };
		ipe.write();
		stream.close();
	}
	break;

	}
}

void write_output_graph_stream(const DiskGraph& graph, std::ostream& stream)
{
	stream << std::setprecision(2);

	for (const auto& v : graph.disks()) {
		const Disk* p = v.parent;
		if (v.failure) {
			stream << "FAILED to place disk " << v.id << " -> " << (p ? p->id : -1) << ".\n";
		}
		else {
			stream << v.id << " -> " << (p ? p->id : -1) << "  (" << v.x << ", " << v.y << ")\n";
		}
	}
}

}
