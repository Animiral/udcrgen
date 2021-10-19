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

/**
 * Used to represent locations on the discrete triangular grid.
 */
struct Coord
{
	int x;   //!< the horizontal x-coordinate
	int sly; //!< the y-coordinate ("slash-y" for slant)

	bool operator==(Coord c) const noexcept;
};

/**
 * Relative step directions on the triangular grid.
 * The direction is not fixed along a particular axis, but oriented along the spine.
 */
enum class Rel { HERE, BACK, BACK_UP, BACK_DOWN, FWD_UP, FWD_DOWN, FORWARD };

using DiskId = int;
constexpr DiskId NODISK = -1;

/**
 * @brief A single unit-sized disk for the output graph representation.
 *
 * It has a unique vertex number within the graph and
 * 2D coordinates to represent the embedding.
 */
struct Disk
{
	// graph info, filled in the classification step - see classify()
	DiskId id; //!< unique vertex number [0..n]
	DiskId parent; //!< parent vertex number, must be spine vertex or -1
	int depth; //!< distance from the spine (0 for spine vertices)

	// embedding info, filled in the embedding step - see embed()
	bool embedded; //!< whether the disk has coordinates or failure
	int grid_x; //!< triangular grid x-coordinate (weak embedding only)
	int grid_sly; //!< triangular grid "slash-y"-coordinate (weak embedding only)
	float x; //!< canvas x-coordinate
	float y; //!< canvas y-coordinate
	bool failure; //!< whether the algorithm failed to place this vertex in UDCR
};