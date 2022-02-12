#include "embed.h"
#include "utility/graph.h"
#include "utility/geometry.h"
#include "utility/util.h"
#include <algorithm>
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

	DiskGraph graph{ spineCount, branchCount, leafCount };

	auto& spineList = graph.spines();

	spineList[0].id = begin[0].from; // this works even with 1 spine, because begin then connects the first branch
	spineList[0].parent = -1;
	spineList[0].depth = 0;
	spineList[0].children = 0;
	spineList[0].failure = false;

	for (int i = 1; i < spineCount; i++) {
		spineList[i].id = begin[i - 1].to;
		spineList[i].parent = begin[i - 1].from;
		spineList[i].depth = 0;
		spineList[i].children = 0;
		spineList[i].failure = false;
	}

	auto& branchList = graph.branches();

	for (int i = 0; i < branchCount; i++) {
		branchList[i].id = branches[i].to;
		branchList[i].parent = branches[i].from;
		graph.findDisk(branchList[i].parent)->children++;
		branchList[i].depth = 1;
		branchList[i].children = 0;
		branchList[i].failure = false;
	}

	// lambda: determine position of disk's parent id in the given list
	auto pos = [](const std::vector<Disk>& l, const Disk& d) {
		return std::find_if(l.begin(), l.end(), [&d](const Disk& p) { return p.id == d.parent; });
	};

	// order branches by index of appearance of their parent in the spine
	std::sort(branchList.begin(), branchList.end(),
		[&l = spineList, pos](const Disk& a, const Disk& b) { return pos(l, a) < pos(l, b); });

	auto& leafList = graph.leaves();

	for (int i = 0; i < leafCount; i++) {
		leafList[i].id = leaves[i].to;
		leafList[i].parent = leaves[i].from;
		graph.findDisk(leafList[i].parent)->children++;
		leafList[i].depth = 2;
		leafList[i].children = 0;
		leafList[i].failure = false;
	}

	// order leaves by index of appearance of their parent in the branches
	std::sort(leafList.begin(), leafList.end(),
		[&l = branchList, pos](const Disk& a, const Disk& b) { return pos(l, a) < pos(l, b); });

	return graph;
}

std::pair<DiskGraph, GraphClass> classify(EdgeList input)
{
	if (input.size() == 0)
		return { {0, 0, 0}, GraphClass::OTHER }; // sanity check for empty graph

	// caterpillar without leaves
	if (recognize_path(input.begin(), input.end())) {
		int const spines = static_cast<int>(input.end() - input.begin()) + 1;
		return { {spines, 0, 0}, GraphClass::CATERPILLAR };
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

	// unfamiliar graph
	return { {0, 0, 0}, GraphClass::OTHER };
}

namespace
{
// Helper functions for embed()

using EmbedOrder = Configuration::EmbedOrder;
using DiskVec = std::vector<Disk>;

void embedDisk(DiskGraph& graph, Embedder& embedder, Disk& disk)
{
	if (disk.embedded)
		return; // nothing to do

	// ensure that parent is embedded
	Disk* parent = graph.findDisk(disk.parent);
	if (parent != nullptr) {
		embedDisk(graph, embedder, *parent);
	}

	embedder.embed(disk);
	disk.embedded = true;
}

std::vector<Disk> getDisksInOrder(const DiskGraph& graph, EmbedOrder embedOrder)
{
	std::vector<Disk> disks;
	disks.reserve(graph.size());

	auto& spines = graph.spines();
	auto& branches = graph.branches(); // branches are ordered by parent spine
	auto& leaves = graph.leaves(); // leaves are ordered by parent branch

	auto spineIt = spines.begin();
	auto branchIt = branches.begin();
	auto leafIt = leaves.begin();

	for (; spineIt != spines.end(); ++spineIt) {
		disks.push_back(*spineIt);

		for (; branchIt != branches.end(); ++branchIt) {
			if (branchIt->parent != spineIt->id)
				break; // skip to next spine

			disks.push_back(*branchIt);

			if (EmbedOrder::DEPTH_FIRST == embedOrder) {
				for (; leafIt != leaves.end(); ++leafIt) {
					if (leafIt->parent != branchIt->id)
						break; // skip to next branch

					disks.push_back(*leafIt);
				}
			}
		}

		if (EmbedOrder::BREADTH_FIRST == embedOrder) {
			for (; leafIt != leaves.end(); ++leafIt) {
				const Disk* branch = graph.findDisk(leafIt->parent);
				assert(branch);

				if (branch->parent != spineIt->id)
					break; // skip to next spine

				disks.push_back(*leafIt);
			}
		}
	}

	return disks;
}

}

using EmbedOrder = Configuration::EmbedOrder;

void embed(DiskGraph& graph, Embedder& embedder, EmbedOrder embedOrder)
{
	auto& spines = graph.spines();
	auto& branches = graph.branches(); // branches are ordered by parent spine
	auto& leaves = graph.leaves(); // leaves are ordered by parent branch

	auto spineIt = spines.begin();
	auto branchIt = branches.begin();
	auto leafIt = leaves.begin();

	for (; spineIt != spines.end(); ++spineIt) {
		embedDisk(graph, embedder, *spineIt);

		for (; branchIt != branches.end(); ++branchIt) {
			if (branchIt->parent != spineIt->id)
				break; // skip to next spine

			embedDisk(graph, embedder, *branchIt);

			if (EmbedOrder::DEPTH_FIRST == embedOrder) {
				for (; leafIt != leaves.end(); ++leafIt) {
					if (leafIt->parent != branchIt->id)
						break; // skip to next branch

					embedDisk(graph, embedder, *leafIt);
				}
			}
		}

		if (EmbedOrder::BREADTH_FIRST == embedOrder) {
			for (; leafIt != leaves.end(); ++leafIt) {
				const Disk* branch = graph.findDisk(leafIt->parent);
				assert(branch);

				if (branch->parent != spineIt->id)
					break; // skip to next spine

				embedDisk(graph, embedder, *leafIt);
			}
		}
	}
}

void embedDynamic(DiskGraph& graph, WholesaleEmbedder& embedder)
{
	std::vector<Disk> disks = getDisksInOrder(graph, EmbedOrder::DEPTH_FIRST);
	embedder.embed(disks);

	// transcribe result to graph
	for (const Disk& out : disks) {
		Disk* actual = graph.findDisk(out.id);
		actual->embedded = out.embedded;
		actual->failure = out.failure;
		actual->grid_x = out.grid_x;
		actual->grid_sly = out.grid_sly;
		actual->x = out.x;
		actual->y = out.y;
	}
}
