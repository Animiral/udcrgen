#include "wudcrgen.h"
#include "graph.h"
#include "geometry.h"
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
DiskGraph fromEdgeList(EdgeList::iterator begin, EdgeList::iterator branches, EdgeList::iterator leaves, EdgeList::iterator end)
{
	const int diskCount = end - begin + 1;
	int spineCount = leaves - begin + 1;

	DiskGraph graph{ diskCount, spineCount };
	auto& disks = graph.disks();

	disks[0].id = begin[0].from;
	disks[0].parent = -1;
	disks[0].failure = false;

	for (int i = 1; i < spineCount; i++) {
		disks[i].id = begin[i - 1].to;
		disks[i].parent = -1;
		disks[i].failure = false;
	}

	// TODO: finish this when the graph representation can handle lobsters.

	return graph;
}

std::pair<DiskGraph, GraphClass> classify(EdgeList input)
{
	if (input.size() == 0)
		return { {0, 0}, GraphClass::OTHER }; // sanity check for empty graph

	// caterpillar without leaves
	if (recognizePath(input.begin(), input.end())) {
		return { {0, 0}, GraphClass::CATERPILLAR };
	}

	auto leaves = separateLeaves(input.begin(), input.end());

	// caterpillar
	if (recognizePath(input.begin(), leaves)) {
		DiskGraph graph = fromEdgeList(input.begin(), leaves, leaves, input.end());
		return { graph, GraphClass::CATERPILLAR };
	}

	auto branches = separateLeaves(input.begin(), leaves);

	// lobster
	if (recognizePath(input.begin(), branches)) {
		DiskGraph graph = fromEdgeList(input.begin(), branches, leaves, input.end());
		return { graph, GraphClass::LOBSTER };
	}

	// unfamiliar graph
	return { {0, 0}, GraphClass::OTHER };
}

/**
 * Cosmetic positioning for disks which cannot be placed by the algorithm.
 */
const float Y_FAIL = 2.2f;

/**
 * Based on a leaf placed previously, find the position for the next one
 * respecting the necessary gap.
 */
Vec2 nextLeafPosition(Vec2 lastLeaf, Vec2 spine, Vec2 lastSpine, Vec2 forward, float gap)
{
	Vec2 leafPosition = triangulate(spine, 1, lastLeaf, 1 + gap, forward, gap * 0.01f);

	if (distance(lastSpine, leafPosition) < 1 + gap) {
		const Vec2 hint = leafPosition - lastSpine;
		leafPosition = triangulate(spine, 1, lastSpine, 1 + gap, hint, gap * 0.01f);
	}

	return leafPosition;
}

DiskGraph udcrgen(const Caterpillar& caterpillar, float gap)
{
	Vec2 position{ 0, 0 }; // embedding position for the current spine piece
	Vec2 forward{ 1.f, 0 }; // general direction for next spine
	Vec2 lastUp{ -10, 1 }; // placement of previous up leaf
	Vec2 lastDown{ -10, -1 }; // placement of previous down leaf
	Vec2 lastSpine{ -1, 0 }; // placement of previous spine
	bool leafUp = false; // where to place the next leaf (alternate)

	auto udcrg = DiskGraph::fromCaterpillar(caterpillar);

	// iterate through all leaves
	int leafIndex = udcrg.spine(); // by convention, leaves start after spine

	// there is a special slot available only to the first leaf on the first spine
	if (leafIndex < udcrg.disks().size()) {
		auto& leaf0 = udcrg.disks()[leafIndex];
		if (0 == leaf0.parent) {
			leaf0.x = -1;
			leaf0.y = 0;
			leafIndex++;
		}
	}

	for (int spineIndex = 0; spineIndex < udcrg.spine(); spineIndex++) {
		// place next spine segment
		auto& spineVertex = udcrg.disks()[spineIndex];
		spineVertex.x = position.x;
		spineVertex.y = position.y;

		// not enough room for the spine (leave it at position anyway)
		if (distance(position, lastUp) < 1 + gap) {
			spineVertex.failure = true;
		}

		// place all leaves for this spine segment
		while (leafIndex < udcrg.disks().size()) {
			auto& leaf = udcrg.disks()[leafIndex];

			if (leaf.parent != spineVertex.id) // need new spine segment
				break;

			auto& lastLeaf = leafUp ? lastUp : lastDown;
			const auto leafPosition = nextLeafPosition(lastLeaf, position, lastSpine, forward, gap);

			// leaf wrap-around collision due to too many leaves
			if (distance(leafPosition, leafUp ? lastDown : lastUp) < 1 + gap) {
				leaf.failure = true;
				leaf.x = position.x;
				leaf.y = position.y + Y_FAIL;
				leafIndex++;
				continue;
			}

			leaf.x = leafPosition.x;
			leaf.y = leafPosition.y;

			leafUp = !leafUp; // alternate placement
			leafIndex++;
			lastLeaf = leafPosition;
		}

		// determine the bisector of the free space ahead as new "forward"
		const auto hypotheticalUp = nextLeafPosition(lastUp, position, lastSpine, forward, gap);
		const auto hypotheticalDown = nextLeafPosition(lastDown, position, lastSpine, forward, gap);
		const auto forward1 = (hypotheticalUp + hypotheticalDown - position - position).unit();
		const auto forward2 = forward1 * -1;
		forward = distance(forward, forward1) < distance(forward, forward2) ? forward1 : forward2;

		// move forward
		lastSpine = position;
		position += forward;
	}

	return udcrg;
}

/**
 * Position names relative to current spine vertex in the weak algorithm.
 */
enum class Slot { BEHIND, UP, DOWN, FWD_UP, FWD_DOWN, FRONT, FAIL };

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

/**
 * Return the corresponding relative slot position.
 */
Vec2 offset(Slot slot) noexcept
{
	return relativeSlots[static_cast<std::size_t>(slot)];
}

/**
 * Return the next slot to place a leaf.
 */
Slot next(Slot slot) noexcept
{
	if (Slot::FAIL == slot)
		return Slot::FAIL;
	else
		return static_cast<Slot>(static_cast<int>(slot) + 1);
}

/**
 * Return the appropriate next slot when advancing in the spine based on the previous position.
 */
Slot atNextSpine(Slot slot)
{
	const int nextInt = static_cast<int>(slot) - 2;
	return static_cast<Slot>(std::max(nextInt, static_cast<int>(Slot::UP)));
}

DiskGraph wudcrgen(const Caterpillar& caterpillar)
{
	// This algorithm places spine disks along the x-axis.
	int slotPosition = 0; // next free slot exists relative to this spine vertex
	Slot slot = Slot::BEHIND;

	auto udcrg = DiskGraph::fromCaterpillar(caterpillar);

	// iterate through all leaves
	int leafIndex = udcrg.spine(); // by convention, leaves start after spine

	for (int spineIndex = 0; spineIndex < udcrg.spine(); spineIndex++) {
		// place next spine segment
		auto& spineVertex = udcrg.disks()[spineIndex];
		spineVertex.x = static_cast<float>(spineIndex);
		spineVertex.y = 0;

		// determine slot for leaves
		if (spineIndex > 0) {
			slot = atNextSpine(slot);
		}

		// place all leaves for this spine segment
		while (leafIndex < udcrg.disks().size()) {
			auto& leaf = udcrg.disks()[leafIndex];

			if (leaf.parent != spineVertex.id) // need new spine segment
				break;

			// fail all leaves for which there is no slot
			if (Slot::FRONT == slot && (spineIndex < udcrg.spine() - 1))
				slot = Slot::FAIL;

			if (Slot::FAIL == slot)
				leaf.failure = true;

			auto leafPosition = Vec2{ spineVertex.x, spineVertex.y };
			leafPosition += offset(slot);
			leaf.x = leafPosition.x;
			leaf.y = leafPosition.y;

			slot = next(slot);
			leafIndex++;
		}
	}

	return udcrg;
}
