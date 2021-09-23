// Geometric types and functions

#pragma once

constexpr float PI = 3.14159265358979323846f;

/**
 * Used to represent points and directions.
 */
struct Vec2
{
	float x;
	float y;

	/**
	 * Return the given vector subtracted from this.
	 */
	Vec2 operator-(Vec2 vec) const noexcept;

	/**
	 * Return this scaled by a factor.
	 */
	Vec2 operator*(float scale) const noexcept;

	/**
	 * Return this, translated.
	 */
	Vec2 operator+(Vec2 translate) const noexcept;

	/**
	 * Apply a translation to this.
	 */
	Vec2& operator+=(Vec2 translate) noexcept;

	/**
	 * Return the length of this vector.
	 */
	float length() const noexcept;

	/**
	 * Return this, scaled to unit length.
	 */
	Vec2 unit() const noexcept;
};

/**
 * Return the straight-line distance between the two points.
 */
float distance(Vec2 from, Vec2 to) noexcept;

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
Vec2 triangulate(Vec2 point0, float distance0, Vec2 point1, float distance1, Vec2 hint, float epsilon) noexcept;
