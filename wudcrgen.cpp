#include "wudcrgen.h"
#include "graph.h"
#include <cmath>
#include <cstdlib>
#include <cassert>

constexpr float PI = 3.14159265358979323846f;

/**
 * Used to represent points and directions.
 */
struct Vec2
{
	float x;
	float y;
};

/**
 * Get the vector from left to right.
 */
constexpr Vec2 operator-(Vec2 left, Vec2 right)
{
	return Vec2{ left.x - right.x, left.y - right.y };
}

/**
 * Get the vector scaled by a factor.
 */
constexpr Vec2 operator*(Vec2 vec, float scale)
{
	return Vec2{ vec.x * scale, vec.y * scale };
}

/**
 * Apply the given translation to the left-side vector.
 */
constexpr Vec2& operator+=(Vec2& vec, Vec2 translate)
{
	vec.x += translate.x;
	vec.y += translate.y;
	return vec;
}

/**
 * Return the translated vector.
 */
Vec2 operator+(Vec2 vec, Vec2 translate)
{
	vec.x += translate.x;
	vec.y += translate.y;
	return vec;
}

/**
 * Return the straight-line distance between the two points.
 */
float distance(Vec2 p0, Vec2 p1)
{
	return std::hypot(p1.x - p0.x, p1.y - p0.y);
}

/**
 * Scale the given vector to unit length.
 */
Vec2 unit(Vec2 vec)
{
	const float d = std::hypot(vec.x, vec.y);
	return Vec2{ vec.x / d, vec.y / d };
}

/**
 * Used to represent rotations for positioning the unit disks.
 */
struct Mat2
{
	float xx; float xy;
	float yx; float yy;

	static Mat2 rotation(float angle)
	{
		return Mat2 {
			std::cos(angle), -std::sin(angle),
			std::sin(angle), std::cos(angle)
		};
	}
};

/**
 * Apply the given transformation matrix to the vector by matrix multiplication.
 */
constexpr Vec2 operator*(Mat2 mat, Vec2 vec)
{
	return {
		mat.xx * vec.x + mat.xy * vec.y,
		mat.yx * vec.x + mat.yy * vec.y
	};
}

/**
 * Multiply the given matrices.
 */
constexpr Mat2 operator*(Mat2 left, Mat2 right)
{
	return {
		left.xx * right.xx + left.xy * right.yx, left.xx * right.xy + left.xy * right.yy,
		left.yx * right.xx + left.yy * right.yx, left.yx * right.xy + left.yy * right.yy
	};
}

/**
 * Determine the location of a new point from two known points and their distance.
 *
 * If the two starting points are too far apart, the result will be between the
 * two, at distance0 from point0.
 *
 * This implementation uses an iteratively converging approach.
 *
 * @param point0: first known point
 * @param distance0: distance from point0 to the target
 * @param point1: second known point
 * @param distance1: distance from point1 to the target
 * @param hint: general preferred direction to discriminate between two solutions
 * @param epsilon: maximum error of result vs solution
 */
Vec2 triangulate(Vec2 point0, float distance0, Vec2 point1, float distance1, Vec2 hint, float epsilon)
{
	Vec2 result; // storage for result

	// if the distance is too far for a matching result, early exit based on point0
	{
		const float totalDistance = distance(point0, point1);
		if (totalDistance >= distance0 + distance1) {
			const Vec2 from0 = (point1 - point0) * (distance0 / totalDistance);
			result = point0 + from0;
			return result;
		}
	}

	// init result to somewhere that is closer to the hinted-at solution.
	result = (point0 + point1) * .5 + hint;

	// distance temporary
	float d = distance(result, point0);

	do {
		const Vec2 from0 = (result - point0) * (distance0 / d);
		result = point0 + from0;

		d = distance(result, point1);
		const Vec2 from1 = (result - point1) * (distance1 / d);
		result = point1 + from1;

		d = distance(result, point0);
	} while (std::abs(d - distance0) > epsilon);

	return result;
}

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

UdcrGraph udcrgen(const Caterpillar& caterpillar, float gap)
{
	Vec2 position{ 0, 0 }; // embedding position for the current spine piece
	Vec2 forward = Mat2::rotation(PI / 12) * Vec2{ 1.f, 0 }; // general direction for next spine
	Vec2 lastUp = { -10, 1 }; // placement of previous up leaf
	Vec2 lastDown = { -10, -1 }; // placement of previous down leaf
	Vec2 lastSpine = { -1, 0 }; // placement of previous spine
	bool leafUp = false; // where to place the next leaf (alternate)

	auto udcrg = UdcrGraph::fromCaterpillar(caterpillar);

	// iterate through all leaves
	int leafIndex = udcrg.spine(); // by convention, leaves start after spine

	// there is a special slot available only to the first leaf on the first spine
	if (leafIndex < udcrg.vertices().size()) {
		auto& leaf0 = udcrg.vertices()[leafIndex];
		if (0 == leaf0.parent) {
			leaf0.x = -1;
			leaf0.y = 0;
			leafIndex++;
		}
	}

	for (int spineIndex = 0; spineIndex < udcrg.spine(); spineIndex++) {
		// place next spine segment
		auto& spineVertex = udcrg.vertices()[spineIndex];
		spineVertex.x = position.x;
		spineVertex.y = position.y;

		// place all leaves for this spine segment
		while (leafIndex < udcrg.vertices().size()) {
			auto& leaf = udcrg.vertices()[leafIndex];

			if (leaf.parent != spineVertex.id) // need new spine segment
				break;

			auto& lastLeaf = leafUp ? lastUp : lastDown;
			const auto leafPosition = nextLeafPosition(lastLeaf, position, lastSpine, forward, gap);
			if (distance(leafPosition, lastLeaf) < 1 + gap) {
				// TODO: raise error - too many leaves to fit
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
		const auto forward1 = unit(hypotheticalUp + hypotheticalDown - position - position);
		const auto forward2 = forward1 * -1;
		forward = distance(forward, forward1) < distance(forward, forward2) ? forward1 : forward2;

		// move forward
		lastSpine = position;
		position += forward;
		if (distance(position, lastUp) < 1 + gap) {
			// TODO: raise error - not enough room for the next spine disk
		}
	}

	return udcrg;
}

/**
 * Position names relative to current spine vertex in the weak algorithm.
 */
enum class Slot { BEHIND, UP, DOWN, FWD_UP, FWD_DOWN, FRONT };

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
	Vec2{ 1, 0 }           // FRONT
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
Slot next(Slot slot)
{
	assert(slot != Slot::FRONT);
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

UdcrGraph wudcrgen(const Caterpillar& caterpillar)
{
	// This algorithm places spine vertices along the x-axis.
	int slotPosition = 0; // next free slot exists relative to this spine vertex
	Slot slot = Slot::BEHIND;

	auto udcrg = UdcrGraph::fromCaterpillar(caterpillar);

	// iterate through all leaves
	int leafIndex = udcrg.spine(); // by convention, leaves start after spine

	for (int spineIndex = 0; spineIndex < udcrg.spine(); spineIndex++) {
		// place next spine segment
		auto& spineVertex = udcrg.vertices()[spineIndex];
		spineVertex.x = static_cast<float>(spineIndex);
		spineVertex.y = 0;

		// determine slot for leaves
		if (spineIndex > 0) {
			slot = atNextSpine(slot);
		}

		// place all leaves for this spine segment
		while (leafIndex < udcrg.vertices().size()) {
			auto& leaf = udcrg.vertices()[leafIndex];

			if (leaf.parent != spineVertex.id) // need new spine segment
				break;

			if (Slot::FRONT == slot && spineIndex < udcrg.spine() - 1) {
				// not enough room for this vertex
				// TODO: raise error
				break;
			}

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
