// Triangular grid representation

#pragma once

#include "geometry.h"
#include <vector>
#include <unordered_map>

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
	 * @a size specifies the maximum number of nodes that can be stored.
	 */
	explicit Grid(std::size_t size);

	/**
	 * Retrieve the disk stored at the given coordinates.
	 */
	Disk* at(Coord coord) const;

	/**
	 * Return the coordinate after taking a step in a particular relative direction.
	 */
	Coord step(Coord from, Rel rel) const;

	/**
	 * Turn the given coordinates into a 2D Euclidean plane vector.
	 */
	Vec2 vec(Coord coord) const;

	/**
	 * Store the given disk reference at the specified coordinates.
	 */
	void put(Coord coord, Disk& disk);

private:

	static std::size_t coordHash(Coord coord) noexcept;

	std::size_t size_;
	std::unordered_map<Coord, Disk*, decltype(&coordHash)> map_;

};
