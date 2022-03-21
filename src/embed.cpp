#include "embed.h"
#include "utility/graph.h"
#include "utility/geometry.h"
#include "utility/util.h"
#include "utility/exception.h"
#include <algorithm>
#include <unordered_map>
#include <stdexcept>

/**
 * Convert a properly pre-processed edge list to a DiskGraph.
 *
 * The edge list must be ordered in the following way:
 *   1. edges which form the spine
 *   2. edges which attach branches
 *   3. edges which attach leaves
 *
 * Edges must point outward, i.e. to the branch or leaf vertex.
 *
 * @param begin beginning of the edge container
 * @param branches beginning of branch-connecting edges
 * @param leaves beginning of leaf-adjacent edges
 * @param end end of the edge container
 */
DiskGraph from_edge_list(EdgeList::iterator begin, EdgeList::iterator branches, EdgeList::iterator leaves, EdgeList::iterator end)
{
	const int spineCount = branches - begin + 1;
	const int branchCount = leaves - branches;
	const int leafCount = end - leaves;

	assert(spineCount > 1 || branchCount > 0); // must have at least one edge
	assert(branchCount > 0 || leafCount == 0); // no leaves without branches

	std::vector<Disk> disks(end - begin + 1);
	std::unordered_map<DiskId, Disk*> lookup;
	std::size_t i = 0;

	disks[i].id = begin[0].from; // this works even with 1 spine, because begin then connects the first branch
	lookup[disks[i].id] = &disks[i];
	disks[i].parent = NODISK;
	disks[i].depth = 0;
	disks[i].children = 0;
	disks[i].failure = false;

	for (i = 1; i < disks.size(); i++) {
		disks[i].id = begin[i - 1].to;
		lookup[disks[i].id] = &disks[i];
		disks[i].parent = begin[i - 1].from;
		
		if (begin + i - 1 >= leaves)
			disks[i].depth = 2;
		else if (begin + i - 1 >= branches)
			disks[i].depth = 1;
		else
			disks[i].depth = 0;

		disks[i].children = 0;
		disks[i].failure = false;
	}

	// fill parents info
	for (i = 1; i < disks.size(); i++) {
		auto it = lookup.find(disks[i].parent);
		
		if (lookup.end() == it)
			throw InputException(format("Faulty graph: node {} (parent of {}) is not connected.", disks[i].parent, disks[i].id));

		Disk* parent = it->second;
		parent->children++;
	}

	DiskGraph result(move(disks));
	result.reorder(Configuration::EmbedOrder::DEPTH_FIRST);
	return result;
}

std::pair<DiskGraph, GraphClass> classify(EdgeList input)
{
	assert(input.size() > 0); // empty graph not allowed

	// caterpillar without leaves
	if (recognize_path(input.begin(), input.end())) {
		DiskGraph graph = from_edge_list(input.begin(), input.end(), input.end(), input.end());
		return { graph, GraphClass::CATERPILLAR };
	}

	auto leaves = separate_leaves(input.begin(), input.end());

	// caterpillar (pretend all leaves are 0-leaf branches)
	if (recognize_path(input.begin(), leaves)) {
		DiskGraph graph = from_edge_list(input.begin(), leaves, input.end(), input.end());
		return { graph, GraphClass::CATERPILLAR };
	}

	auto branches = separate_leaves(input.begin(), leaves);

	// lobster
	if (recognize_path(input.begin(), branches)) {
		// "leaves" are actually 0-leaf branches to us if they connect to the spine
		auto isSpine = [&input, branches](DiskId id)
		{
			return input[0].from == id ||
				std::any_of(input.begin(), branches, [id](Edge e) { return e.to == id; });
		};
		leaves = std::partition(branches, input.end(), [&isSpine](Edge e) { return isSpine(e.from); });

		DiskGraph graph = from_edge_list(input.begin(), branches, leaves, input.end());
		return { graph, GraphClass::LOBSTER };
	}

	throw InputException("Unrecognized graph type.");
}

Stat embed(DiskGraph& graph, Embedder& embedder, Configuration::Algorithm algorithm, Configuration::EmbedOrder embedOrder)
{
	using Clock = std::chrono::steady_clock;
	Clock clock;
	Clock::time_point start;

	Stat stat;
	stat.size = graph.size();
	stat.spines = graph.length();
	stat.algorithm = algorithm;
	start = clock.now();

	// timed instructions
	{
		graph.reorder(embedOrder);
		embedder.setGraph(graph);

		stat.success = true;

		for (Disk& disk : graph.disks()) {
			if (!disk.embedded) {
				embedder.embed(disk);
				stat.success &= !disk.failure;
			}
		}
	}

	stat.duration = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now() - start);
	return stat;
}

Stat embedDynamic(DiskGraph& graph, WholesaleEmbedder& embedder)
{
	using Clock = std::chrono::steady_clock;
	Clock clock;
	Clock::time_point start;

	Stat stat;
	stat.size = graph.size();
	stat.spines = graph.length();
	stat.algorithm = Configuration::Algorithm::DYNAMIC_PROGRAM;
	start = clock.now();

	// timed instructions
	{
		graph.reorder(Configuration::EmbedOrder::DEPTH_FIRST);
		stat.success = embedder.embed(graph.disks());
	}

	stat.duration = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now() - start);
	return stat;
}
