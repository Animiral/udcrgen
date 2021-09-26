#include "geometry.h"
#include <cmath>
#include <cstdlib>

Vec2 Vec2::operator-(Vec2 vec) const noexcept
{
	return { x - vec.x, y - vec.y };
}

Vec2 Vec2::operator*(float scale) const noexcept
{
	return { x * scale, y * scale };
}

Vec2 Vec2::operator+(Vec2 translate) const noexcept
{
	return { x + translate.x, y + translate.y };
}

Vec2& Vec2::operator+=(Vec2 translate) noexcept
{
	x += translate.x;
	y += translate.y;
	return *this;
}

float Vec2::length() const noexcept
{
	return std::hypot(x, y);
}

Vec2 Vec2::unit() const noexcept
{
	const float d = length();
	return { x / d, y / d };
}

float distance(Vec2 from, Vec2 to) noexcept
{
	return (to - from).length();
}

Vec2 triangulate(Vec2 point0, float distance0, Vec2 point1, float distance1, Vec2 hint, float epsilon) noexcept
{
	Vec2 result; // storage for result

	// if the distance is too far for a matching result, early exit based on point0
	{
		const Vec2 v01 = point1 - point0;
		const float totalDistance = v01.length();
		if (totalDistance >= distance0 + distance1) {
			const Vec2 from0 = v01 * (distance0 / totalDistance);
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
