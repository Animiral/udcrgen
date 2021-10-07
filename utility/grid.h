// Triangular grid representation

#pragma once

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
	 * Read- or write-access the value stored at the given coordinates.
	 *
	 * @arg @c x the x-coordinate of the index to access
	 * @arg @c sly the y-coordinate ("slash-y" for slant) of the index to access
	 */
	int& at(int x, int sly);

private:

	int length_;
	int size_;
	std::vector<int> values_;

};
