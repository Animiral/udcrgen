#include "wudcrgen.h"
#include "graph.h"
#include <cmath>
#include <cassert>

constexpr auto PI = 3.14159265358979323846;

/**
 * Used to represent points and directions.
 */
struct Vec2
{
	float x;
	float y;
};

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

UdcrGraph udcrgen(const Caterpillar& caterpillar)
{
	constexpr float sixtyDegrees = PI / 3.f;

	Vec2 position{ 0, 0 }; // embedding position for the current spine piece
	Vec2 forward = Mat2::rotation(sixtyDegrees/4) * Vec2{ 1.f, 0 }; // general direction for next spine
	float slotUp = 2 * sixtyDegrees; // rotation to place leaf up
	float slotDown = -3 * sixtyDegrees; // rotation to place leaf down
	bool leafUp = false; // where to place the next leaf (alternate)

	auto udcrg = UdcrGraph::fromCaterpillar(caterpillar);

	// iterate through all leaves
	int leafIndex = udcrg.spine(); // by convention, leaves start after spine

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

			auto leafPosition = position;

			if (leafUp) {
				leafPosition += Mat2::rotation(slotUp) * forward;
				slotUp -= sixtyDegrees;
			}
			else {
				leafPosition += Mat2::rotation(slotDown) * forward;
				slotDown += sixtyDegrees;
			}
			leaf.x = leafPosition.x;
			leaf.y = leafPosition.y;

			leafUp = !leafUp; // alternate placement
			leafIndex++;
		}

		// determine the bisector of the free space ahead as new "forward"
		const auto forwardDirection = (slotUp + slotDown) / 2;
		forward = Mat2::rotation(forwardDirection) * forward;

		// determine the tightest possible placement slots for upcoming leaves
		// due to unit circles, the total angle is the same, but split evenly
		slotUp = std::min((slotUp - slotDown) / 2 + sixtyDegrees, 2 * sixtyDegrees);
		slotDown = -slotUp;

		// move forward
		position += forward;
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
