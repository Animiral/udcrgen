#include "wudcrgen.h"
#include "graph.h"
#include <cmath>

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

UdcrGraph wudcrgen(const Caterpillar& caterpillar)
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
