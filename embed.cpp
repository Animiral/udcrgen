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

	assert(spineCount > 1 || branchCount > 0); // must have at least one edge
	assert(branchCount > 0 || leafCount == 0); // no leaves without branches

	DiskGraph graph{ spineCount, branchCount, leafCount };

	auto& spineList = graph.spines();

	spineList[0].id = begin[0].from; // this works even with 1 spine, because begin then connects the first branch
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

	// "leaves" are actually 0-leaf branches to us if they connect to the spine
	auto isSpine = [&input, branches](int id)
	{
		return input[0].from == id ||
			std::any_of(input.begin(), branches, [id](Edge e) { return e.to == id; });
	};
	leaves = std::partition(branches, input.end(), [&isSpine](Edge e) { return isSpine(e.from); });

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

EmbedResult ProperEmbedder::branch() noexcept
{
	assert(!beforeFirstSpine_);

	// We still refer to branches as "leaves" here because the proper
	// algorithm does not yet handle lobsters.
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

EmbedResult ProperEmbedder::leaf() noexcept
{
	// This embedder cannot yet deal with lobster leaves.
	return { {0, Y_FAIL}, true };
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
	locality_{ (int)Rel::SPINE },
	affinity_{ Affinity::UP },
	spineIndex_{ -1 },
	spineCount_{ spineCount }
{
	std::fill(zone_, zone_ + 25, false);

	// reserve space for spines, if necessary
	for (int i = 0; i < spineCount && i < 2; i++) {
		zone_[(int)Rel::SPINE + (int)Rel::FRONT * (i+1)] = true;
	}
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

	// shift all contextual information in the zone
	auto end = std::copy(zone_ + 5, zone_ + 25, zone_);
	std::fill(zone_ + 20, zone_ + 25, false);

	// Reserve space for straight spine ahead
	if (spineIndex_ + 2 < spineCount_)
		zone_[22] = true;

	return { { static_cast<float>(spineIndex_), 0 }, false }; // straight spine
}

EmbedResult WeakEmbedder::branch() noexcept
{
	// invert affinity for even distribution
	affinity_ = (Affinity)((int)affinity_ * -1);

	static const int spineLocality = (int)Rel::SPINE;
	const int position = findFreePosition(spineLocality, affinity_);

	if (position >= 0) {
		zone_[position] = true;
		locality_ = position;
		return { getCoords(position), false };
	}
	else {
		return { getCoords(spineLocality) + Vec2{ 0, Y_FAIL }, true };
	}
}

EmbedResult WeakEmbedder::leaf() noexcept
{
	const int position = findFreePosition(locality_, affinity_);

	if (position >= 0) {
		zone_[position] = true;
		return { getCoords(position), false };
	}
	else {
		return { getCoords(locality_) + Vec2{ 0, Y_FAIL }, true };
	}
}

int WeakEmbedder::findFreePosition(int locality, Affinity affinity) noexcept
{
	static const Rel upCandidates[6] = { Rel::BEHIND, Rel::UP, Rel::FWD_UP, Rel::FRONT, Rel::FWD_DOWN, Rel::DOWN };
	static const Rel downCandidates[6] = { Rel::BEHIND, Rel::DOWN, Rel::FWD_DOWN, Rel::FRONT, Rel::FWD_UP, Rel::UP };
	auto* const candidates = Affinity::UP == affinity ? upCandidates : downCandidates;
	auto* const end = candidates + 6;

	auto it = std::find_if(candidates, end, [this, locality](Rel r) { return !zone_[locality + (int)r]; });
	if (it == end)
		return -1; // no slot free

	return locality + (int)*it;
}

Vec2 WeakEmbedder::getCoords(int position) noexcept
{
	const int forwardSteps = position / 5 - 2;
	const int upSteps = -(position % 5) + 2;

	return { spineIndex_ + forwardSteps + upSteps * .5f, upSteps * Y_HIGH };
}

void embed(DiskGraph& graph, Embedder& embedder)
{
	auto& spines = graph.spines();
	auto& branches = graph.branches();
	auto& leaves = graph.leaves();

	auto branchIt = branches.begin(); // branches are ordered by parent spine
	auto leafIt = leaves.begin(); // leaves are ordered by parent branch

	for (Disk& spineDisk : spines) {
		// place next spine segment
		embedder.spine().applyTo(spineDisk);

		for (; branchIt != branches.end() && branchIt->parent == spineDisk.id; ++branchIt) {
			embedder.branch().applyTo(*branchIt);

			for (; leafIt != leaves.end() && leafIt->parent == branchIt->id; ++leafIt) {
				embedder.leaf().applyTo(*leafIt);
			}
		}
	}
}
