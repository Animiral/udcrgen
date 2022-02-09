#pragma once

#include <bitset>
#include "utility/graph.h"
#include "utility/grid.h"
#include "utility/geometry.h"
#include "config.h"

/**
 * @brief Represents a local fundament, which is inherently limited to
 *        coordinates reachable from some spine point of interest within two steps.
 *
 * The representation uses a bitmask in which the bit number <tt>n = (sly+x+2)*5 + (x+2)</tt>
 * is set to @c true if the grid location <tt>(x,sly): (sly+x) &#8712; [-2,2], x &#8712; [-2,2]</tt>
 * relative to the spine head is blocked, @c false if it is free.
 */
struct Fundament
{
	std::bitset<25> mask;

	bool blocked(Coord c) const noexcept;
};

Fundament makeFundament(Grid grid, Coord head);

using InputDisks = std::vector<Disk>; // complete input

/**
 * An instance of the dynamic programming problem.
 *
 * It consists of the solution so far and an iterator
 * into the ordered set of disks yet to solve.
 */
class DynamicProblem
{

public:

	explicit DynamicProblem(InputDisks& input);

private:

	DynamicProblem(InputDisks& input, Grid solution, Coord spineHead, Coord branchHead);

public:

	std::vector<DynamicProblem> subproblems() const;

	void setSolution(Grid solution, Coord spineHead, Coord branchHead);

	const Grid& solution() const noexcept;
	Coord spineHead() const noexcept;
	Coord branchHead() const noexcept;
	int depth() const noexcept;

private:

	Grid solution_; // disks embedded so far
	Coord spineHead_;
	Coord branchHead_;
	InputDisks* input_;
	int depth_;

};
