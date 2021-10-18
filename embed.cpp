#include "embed.h"
#include "utility/graph.h"
#include "utility/geometry.h"
#include "utility/util.h"
#include <algorithm>
#include <stdexcept>
#include <cassert>

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
	spineList[0].failure = false;

	for (int i = 1; i < spineCount; i++) {
		spineList[i].id = begin[i - 1].to;
		spineList[i].parent = begin[i - 1].from;
		spineList[i].depth = 0;
		spineList[i].failure = false;
	}

	auto& branchList = graph.branches();

	for (int i = 0; i < branchCount; i++) {
		branchList[i].id = branches[i].to;
		branchList[i].parent = branches[i].from;
		branchList[i].depth = 1;
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
		leafList[i].depth = 2;
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

/**
 * Cosmetic positioning for disks which cannot be placed by the algorithm.
 */
const float Y_FAIL = 2.2f;

ProperEmbedder::ProperEmbedder() noexcept :
	spine_{ 0, 0 },
	forward_{ 1.f, 0 },
	lastUp_{ -10, 1 },
	lastDown_{ -10, -1 },
	lastSpine_{ -1, 0 },
	leafUp_{ false },
	gap_{ 0.1f },
	beforeFirstSpine_{ true },
	atFirstSpine_{ false },
	beforeFirstLeaf_{ true }
{
}

void ProperEmbedder::setGap(float gap) noexcept
{
	gap_ = gap;
}

void ProperEmbedder::embed(Disk& disk)
{
	if (0 == disk.depth)
		return embedSpine(disk);

	if (1 == disk.depth)
		return embedLeaf(disk);

	else
		throw std::exception("proper embedder can not embed graphs deeper than caterpillars");
}

void ProperEmbedder::embedSpine(Disk& disk) noexcept
{
	if (beforeFirstSpine_) {
		beforeFirstSpine_ = false;
		disk.x = spine_.x;
		disk.y = spine_.y;
		return;
	}

	// determine the bisector of the free space ahead as new "forward"
	const auto hypotheticalUp = findLeafPosition(lastUp_);
	const auto hypotheticalDown = findLeafPosition(lastDown_);
	const auto forward1 = (hypotheticalUp + hypotheticalDown - spine_ - spine_).unit();
	const auto forward2 = forward1 * -1;
	forward_ = distance(forward_, forward1) < distance(forward_, forward2) ? forward1 : forward2;

	// move forward
	lastSpine_ = spine_;
	spine_ += forward_;
	beforeFirstLeaf_ = false;

	disk.x = spine_.x;
	disk.y = spine_.y;
	disk.failure = distance(spine_, lastUp_) < 1 + gap_; // is there enough room for the spine?
}

void ProperEmbedder::embedLeaf(Disk& disk) noexcept
{
	assert(!beforeFirstSpine_);

	if (beforeFirstLeaf_) {
		beforeFirstLeaf_ = false;
		disk.x = -1;
		disk.y = 0;
		return;
	}

	Vec2& lastLeaf = leafUp_ ? lastUp_ : lastDown_;
	Vec2 leafPosition = findLeafPosition(lastLeaf);

	// leaf wrap-around collision due to too many leaves
	if (distance(leafPosition, leafUp_ ? lastDown_ : lastUp_) < 1 + gap_) {
		leafPosition.y += Y_FAIL;
		disk.x = leafPosition.x;
		disk.y = leafPosition.y;
		disk.failure = true;
		return;
	}

	leafUp_ = !leafUp_; // alternate placement
	lastLeaf = leafPosition;

	disk.x = leafPosition.x;
	disk.y = leafPosition.y;
}

Vec2 ProperEmbedder::findLeafPosition(Vec2 constraint) noexcept
{
	Vec2 leafPosition = triangulate(spine_, 1, constraint, 1 + gap_, forward_, gap_ * 0.01f);

	// If the position under the argument constraint is too close to the last spine,
	// try again using the last spine as a constraint.
	if (distance(lastSpine_, leafPosition) < 1 + gap_) {
		const Vec2 hint = leafPosition - lastSpine_;
		leafPosition = triangulate(spine_, 1, lastSpine_, 1 + gap_, hint, gap_ * 0.01f);
	}

	return leafPosition;
}

WeakEmbedder::WeakEmbedder(int spineCount) noexcept :
	grid_{ spineCount, 2 }
{
}

/**
 * Neighbors are placed in-between in the rows above and below.
 */
const float Y_HIGH = std::sqrt(3.f) / 2;

/**
 * Relative positions by slot name.
 */
const Vec2 relativeSlots[] = {
	Vec2{ -1, 0 },         // BEHIND
	Vec2{ -.5, Y_HIGH },   // UP
	Vec2{ -.5, -Y_HIGH },  // DOWN
	Vec2{ .5, Y_HIGH },    // FWD_UP
	Vec2{ .5, -Y_HIGH },   // FWD_DOWN
	Vec2{ 1, 0 },          // FRONT
	Vec2{ 0, Y_FAIL }      // FAIL
};

void WeakEmbedder::embed(Disk& disk)
{
	if (0 == disk.depth)
		return embedSpine(disk);

	if (1 == disk.depth || 2 == disk.depth)
		return embedBranchOrLeaf(disk);

	else
		throw std::exception("weak embedder can not embed graphs deeper than lobsters");
}

void WeakEmbedder::embedSpine(Disk& disk) noexcept
{
	Coord coord{ 0, 0 };

	if(-1 != disk.parent)
	{
		Coord parentCoord =  grid_.find(disk.parent);
		coord = grid_.step(parentCoord, Rel::FORWARD);
	}

	if (NODISK == grid_.at(coord)) {
		putDiskAt(disk, coord);
	}
	else {
		disk.failure = true;
	}

	if (disk.failure) {
		trace("FAIL spine");
	}
	else {
		trace("Embed spine at ({}/{})", disk.x, disk.y);
	}
}

void WeakEmbedder::embedBranchOrLeaf(Disk& disk) noexcept
{
	assert(NODISK != disk.parent); // branches and leaves always have parents

	Coord parentCoord = grid_.find(disk.parent);
	Affinity affinity = determineAffinity(parentCoord); // whether to place disk high or low

	putDiskNear(disk, parentCoord, affinity);

	if (disk.failure) {
		trace(1 == disk.depth ? "FAIL branch" : "FAIL leaf");
	}
	else {
		trace(1 == disk.depth ? "Embed branch at ({}/{})" : "Embed leaf at ({}/{})", disk.x, disk.y);
	}
}

void WeakEmbedder::putDiskNear(Disk& disk, Coord coord, Affinity affinity) noexcept
{
	static const Rel upCandidates[6] = { Rel::BACK, Rel::BACK_UP, Rel::FWD_UP, Rel::FORWARD, Rel::FWD_DOWN, Rel::BACK_DOWN };
	static const Rel downCandidates[6] = { Rel::BACK, Rel::BACK_DOWN, Rel::FWD_DOWN, Rel::FORWARD, Rel::FWD_UP, Rel::BACK_UP };

	auto* const candidates = Affinity::UP == affinity ? upCandidates : downCandidates;
	auto* const end = candidates + 6;

	for (int i = 0; i < 6; i++) {
		Coord target = grid_.step(coord, candidates[i]);

		if (NODISK == grid_.at(target)) {
			putDiskAt(disk, target);
			return;
		}
	}

	// no free coordinate found
	disk.failure = true;
}

void WeakEmbedder::putDiskAt(Disk& disk, Coord coord) noexcept
{
	grid_.put(coord, disk.id);
	disk.grid_x = coord.x;
	disk.grid_sly = coord.sly;
	Vec2 diskVec = grid_.vec(coord);
	disk.x = diskVec.x;
	disk.y = diskVec.y;
}

WeakEmbedder::Affinity WeakEmbedder::determineAffinity(Coord center) noexcept
{
	// affinity is based on the available free space in the vicinity
	Coord upperArea[] = {
		grid_.step(center, Rel::BACK_UP),
		grid_.step(grid_.step(center, Rel::BACK_UP), Rel::BACK),
		grid_.step(grid_.step(center, Rel::BACK_UP), Rel::BACK_UP),
		grid_.step(grid_.step(center, Rel::BACK_UP), Rel::FWD_UP),
		grid_.step(center, Rel::FWD_UP),
		grid_.step(grid_.step(center, Rel::FWD_UP), Rel::FWD_UP),
		grid_.step(grid_.step(center, Rel::FWD_UP), Rel::FORWARD)
	};

	Coord lowerArea[] = {
		grid_.step(center, Rel::BACK_DOWN),
		grid_.step(grid_.step(center, Rel::BACK_DOWN), Rel::BACK),
		grid_.step(grid_.step(center, Rel::BACK_DOWN), Rel::BACK_DOWN),
		grid_.step(grid_.step(center, Rel::BACK_DOWN), Rel::FWD_DOWN),
		grid_.step(center, Rel::FWD_DOWN),
		grid_.step(grid_.step(center, Rel::FWD_DOWN), Rel::FWD_DOWN),
		grid_.step(grid_.step(center, Rel::FWD_DOWN), Rel::FORWARD)
	};

	int upperWeight = 0;
	int lowerWeight = 0;

	for (int i = 0; i < 7; i++) {
		upperWeight += NODISK != grid_.at(upperArea[i]);
		lowerWeight += NODISK != grid_.at(lowerArea[i]);
	}

	return lowerWeight < upperWeight ? Affinity::DOWN : Affinity::UP;
}

namespace
{
// Helper functions for embed()

using EmbedOrder = Configuration::EmbedOrder;
using DiskVec = std::vector<Disk>;

/**
 * Return the disk vectors to embed by their priority according to the given embed order.
 */
std::tuple<DiskVec*, DiskVec*, DiskVec*> decodePriority(DiskGraph& graph, EmbedOrder embedOrder)
{
	auto* spines = &graph.spines(); // branches are ordered by parent spine
	auto* branches = &graph.branches(); // branches are ordered by parent spine
	auto* leaves = &graph.leaves(); // leaves are ordered by parent branch

	using std::tie;

	switch (embedOrder) {
	case EmbedOrder::LBS: return tie(leaves, branches, spines);
	case EmbedOrder::BLS: return tie(branches, leaves, spines);
	case EmbedOrder::LSB: return tie(leaves, spines, branches);
	case EmbedOrder::BSL: return tie(branches, spines, leaves);
	case EmbedOrder::SBL: return tie(spines, branches, leaves);
	case EmbedOrder::SLB: return tie(spines, leaves, branches);
	default: assert(0); return {};
	}
}

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

}

using EmbedOrder = Configuration::EmbedOrder;

void embed(DiskGraph& graph, Embedder& embedder, EmbedOrder embedOrder)
{
	DiskVec *p1, *p2, *p3;
	std::tie(p1, p2, p3) = decodePriority(graph, embedOrder);

	for (Disk& disk : *p1) embedDisk(graph, embedder, disk);
	for (Disk& disk : *p2) embedDisk(graph, embedder, disk);
	for (Disk& disk : *p3) embedDisk(graph, embedder, disk);
}
