// Triangular grid representation

#pragma once

#include "geometry.h"
#include <vector>

/**
 * This triangular grid is used for the weak contact lobster embedding.
 * The grid stores integers, which are used for disk IDs.
 */
class Grid
{

public:

	/**
	 * Initialize the grid to support the given size.
	 *
	 * @a length specifies the number of center points (spine nodes).
	 * Any coordinate reachable within @a size hops of a center is
	 * part of the grid.
	 */
	Grid(int length, int size);

	/**
	 * Read the value stored at the given coordinates.
	 */
	DiskId at(Coord coord) const;

	/**
	 * Return the unique placement location of the given disk.
	 */
	Coord find(DiskId id) const;

	/**
	 * Return the coordinate after taking a step in a particular relative direction.
	 */
	Coord step(Coord from, Rel rel) const;

	/**
	 * Turn the given coordinates into a 2D Euclidean plane vector.
	 */
	Vec2 vec(Coord coord) const;

	/**
	 * Store the given disk ID at the specified coordinates.
	 */
	void put(Coord coord, DiskId id);

private:

	int length_;
	int size_;
	std::vector<DiskId> values_;

	std::size_t coordIndex(Coord coord) const;

};
