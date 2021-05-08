#include "embed.h"
#include "graph.h"
#include "geometry.h"
#include <algorithm>
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

	DiskGraph graph{ spineCount, branchCount, leafCount };

	auto& spineList = graph.spines();

	spineList[0].id = begin[0].from;
	spineList[0].parent = -1;
	spineList[0].failure = false;

	for (int i = 1; i < spineCount; i++) {
		spineList[i].id = begin[i - 1].to;
		spineList[i].parent = -1;
		spineList[i].failure = false;
	}

	auto& branchList = graph.branches();

	for (int i = 0; i < branchCount; i++) {
		branchList[i].id = branches[i].to;
		branchList[i].parent = branches[i].from;
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
		return { {0, 0, 0}, GraphClass::CATERPILLAR };
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
		DiskGraph graph = from_edge_list(input.begin(), branches, leaves, input.end());
		return { graph, GraphClass::LOBSTER };
	}

	// unfamiliar graph
	return { {0, 0, 0}, GraphClass::OTHER };
}

void EmbedResult::applyTo(Disk& disk) noexcept
{
	disk.x = position.x;
	disk.y = position.y;
	disk.failure = failure;
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
	beforeFirstLeaf_{ true }
{
}

void ProperEmbedder::setGap(float gap) noexcept
{
	gap_ = gap;
}

EmbedResult ProperEmbedder::spine() noexcept
{
	if (beforeFirstSpine_) {
		beforeFirstSpine_ = false;
		return { spine_, false };
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

	// is there enough room for the spine?
	bool failure = distance(spine_, lastUp_) < 1 + gap_;

	return { spine_, failure };
}

EmbedResult ProperEmbedder::leaf() noexcept
{
	assert(!beforeFirstSpine_);

	if (beforeFirstLeaf_) {
		beforeFirstLeaf_ = false;
		return { {-1, 0}, false };
	}

	Vec2& lastLeaf = leafUp_ ? lastUp_ : lastDown_;
	Vec2 leafPosition = findLeafPosition(lastLeaf);

	// leaf wrap-around collision due to too many leaves
	if (distance(leafPosition, leafUp_ ? lastDown_ : lastUp_) < 1 + gap_) {
		leafPosition.y += Y_FAIL;
		return { leafPosition, true };
	}

	leafUp_ = !leafUp_; // alternate placement
	lastLeaf = leafPosition;

	return { leafPosition, false };
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
	slot_{ Slot::BEHIND },
	spineIndex_{ -1 },
	spineCount_{ spineCount }
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

EmbedResult WeakEmbedder::spine() noexcept
{
	spineIndex_++;

	// adjust slot relative to next spine
	if (0 == spineIndex_) {
		slot_ = Slot::BEHIND;
	}
	else {
		const int nextSlot = static_cast<int>(slot_) - 2;
		slot_ = static_cast<Slot>(std::max(nextSlot, static_cast<int>(Slot::UP)));
	}

	return { { static_cast<float>(spineIndex_), 0 }, false }; // straight spine
}

EmbedResult WeakEmbedder::leaf() noexcept
{
	// the front slot is only available if there are no more spines coming
	if (Slot::FRONT == slot_ && (spineIndex_ < spineCount_ - 1))
		slot_ = Slot::FAIL;

	Vec2 leafPosition = Vec2{ static_cast<float>(spineIndex_), 0 } +
		relativeSlots[static_cast<std::size_t>(slot_)];
	bool failure = Slot::FAIL == slot_;

	// next slot
	if (WeakEmbedder::Slot::FAIL != slot_)
		slot_ = static_cast<WeakEmbedder::Slot>(static_cast<int>(slot_) + 1);

	return { leafPosition, failure };
}

void embed(DiskGraph& graph, Embedder& embedder)
{
	auto& spines = graph.spines();
	auto& branches = graph.branches(); // TODO: support lobsters
	auto& leaves = graph.leaves();

	auto branchIt = branches.begin(); // branches are ordered by parent spine

	for (Disk& spineDisk : spines) {
		// place next spine segment
		embedder.spine().applyTo(spineDisk);

		for (; branchIt != branches.end() && branchIt->parent == spineDisk.id; ++branchIt) {
			embedder.leaf().applyTo(*branchIt);
		}
	}
}
