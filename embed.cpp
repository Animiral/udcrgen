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


GridEmbedImpl::GridEmbedImpl(size_t size) noexcept
	: principalDirection(Dir::RIGHT), grid_(size)
{
}

const Grid& GridEmbedImpl::grid() const noexcept
{
	return grid_;
}

GridEmbedImpl::Affinity GridEmbedImpl::determineAffinity(Coord center) const noexcept
{
	// affinity is based on the available free space in the vicinity
	Coord upperArea[] = {
		grid_.step(center, principalDirection, Rel::BACK_UP),
		grid_.step(grid_.step(center, principalDirection, Rel::BACK_UP), principalDirection, Rel::BACK),
		grid_.step(grid_.step(center, principalDirection, Rel::BACK_UP), principalDirection, Rel::BACK_UP),
		grid_.step(grid_.step(center, principalDirection, Rel::BACK_UP), principalDirection, Rel::FWD_UP),
		grid_.step(center, principalDirection, Rel::FWD_UP),
		grid_.step(grid_.step(center, principalDirection, Rel::FWD_UP), principalDirection, Rel::FWD_UP),
		grid_.step(grid_.step(center, principalDirection, Rel::FWD_UP), principalDirection, Rel::FORWARD)
	};

	Coord lowerArea[] = {
		grid_.step(center, principalDirection, Rel::BACK_DOWN),
		grid_.step(grid_.step(center, principalDirection, Rel::BACK_DOWN), principalDirection, Rel::BACK),
		grid_.step(grid_.step(center, principalDirection, Rel::BACK_DOWN), principalDirection, Rel::BACK_DOWN),
		grid_.step(grid_.step(center, principalDirection, Rel::BACK_DOWN), principalDirection, Rel::FWD_DOWN),
		grid_.step(center, principalDirection, Rel::FWD_DOWN),
		grid_.step(grid_.step(center, principalDirection, Rel::FWD_DOWN), principalDirection, Rel::FWD_DOWN),
		grid_.step(grid_.step(center, principalDirection, Rel::FWD_DOWN), principalDirection, Rel::FORWARD)
	};

	int upperWeight = 0;
	int lowerWeight = 0;

	for (int i = 0; i < 7; i++) {
		upperWeight += nullptr != grid_.at(upperArea[i]);
		lowerWeight += nullptr != grid_.at(lowerArea[i]);
	}

	return lowerWeight < upperWeight ? Affinity::DOWN : Affinity::UP;
}

Dir GridEmbedImpl::determinePrincipal(Coord tip) const noexcept
{
	return Dir::RIGHT;
}

int GridEmbedImpl::countFreeNeighbors(Coord center) const noexcept
{
	Coord neighbors[] = {
		grid_.step(center, principalDirection, Rel::BACK),
		grid_.step(center, principalDirection, Rel::BACK_UP),
		grid_.step(center, principalDirection, Rel::BACK_DOWN),
		grid_.step(center, principalDirection, Rel::FWD_UP),
		grid_.step(center, principalDirection, Rel::FWD_DOWN),
		grid_.step(center, principalDirection, Rel::FORWARD)
	};

	return std::count_if(neighbors, neighbors + 6,
		[this](Coord c) { return !grid_.at(c); });
}

void GridEmbedImpl::putDiskNear(Disk& disk, Coord coord, Affinity affinity) noexcept
{
	static const Rel upCandidates[6] = { Rel::BACK, Rel::BACK_UP, Rel::FWD_UP, Rel::FORWARD, Rel::FWD_DOWN, Rel::BACK_DOWN };
	static const Rel downCandidates[6] = { Rel::BACK, Rel::BACK_DOWN, Rel::FWD_DOWN, Rel::FORWARD, Rel::FWD_UP, Rel::BACK_UP };

	const Rel *candidates, *end;
	
	if (0 == disk.depth) {
		static const Rel spineRel = Rel::FORWARD;
		candidates = &spineRel;
		end = candidates + 1;
	}
	else {
		if (Affinity::UP == affinity)
			candidates = upCandidates;
		else
			candidates = downCandidates;

		end = candidates + 6;
	}

	for (; candidates != end; ++candidates) {
		Coord target = grid_.step(coord, principalDirection, *candidates);

		if (!grid_.at(target) &&
			// space heuristic: we must leave space for leaves
			countFreeNeighbors(target) >= disk.children) {

			putDiskAt(disk, target);
			return;
		}
	}

	// no free coordinate found
	disk.failure = true;
}

void GridEmbedImpl::putDiskAt(Disk& disk, Coord coord) noexcept
{
	grid_.put(coord, disk);
	disk.grid_x = coord.x;
	disk.grid_sly = coord.sly;
	Vec2 diskVec = grid_.vec(coord);
	disk.x = diskVec.x;
	disk.y = diskVec.y;
}


WeakEmbedder::WeakEmbedder(DiskGraph& graph) noexcept :
	graph_(&graph), impl_{ graph.size() }
{
	// sync grid to graph state
	for (Disk& disk : graph.spines()) {
		if (disk.embedded)
			impl_.putDiskAt(disk, { disk.grid_x, disk.grid_sly });
	}
	for (Disk& disk : graph.branches()) {
		if (disk.embedded)
			impl_.putDiskAt(disk, { disk.grid_x, disk.grid_sly });
	}
	for (Disk& disk : graph.leaves()) {
		if (disk.embedded)
			impl_.putDiskAt(disk, { disk.grid_x, disk.grid_sly });
	}
}

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
		const Disk* parent = graph_->findDisk(disk.parent);
		assert(parent);
		Coord parentCoord{ parent->grid_x, parent->grid_sly };
		coord = impl_.grid().step(parentCoord, impl_.principalDirection, Rel::FORWARD);
	}

	if (impl_.grid().at(coord)) {
		disk.failure = true;
		trace("FAIL spine id {}", disk.id);
	}
	else {
		impl_.putDiskAt(disk, coord);
		trace("Embed spine id {} at ({}/{})", disk.id, disk.x, disk.y);
	}
}

void WeakEmbedder::embedBranchOrLeaf(Disk& disk) noexcept
{
	assert(NODISK != disk.parent); // branches and leaves always have parents

	const Disk* parent = graph_->findDisk(disk.parent);
	assert(parent);
	Coord parentCoord{ parent->grid_x, parent->grid_sly };
	GridEmbedImpl::Affinity affinity = impl_.determineAffinity(parentCoord); // whether to place disk high or low

	impl_.putDiskNear(disk, parentCoord, affinity);

	if (disk.failure) {
		trace(1 == disk.depth ? "FAIL branch id {}" : "FAIL leaf id {}", disk.id);
	}
	else {
		trace(1 == disk.depth ? "Embed branch id {} at ({}/{})" : "Embed leaf id {} at ({}/{})", disk.id, disk.x, disk.y);
	}
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
