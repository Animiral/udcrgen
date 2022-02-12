// Embedding algorithms based on dynamic programming

#pragma once

#include <bitset>
#include <queue>
#include <unordered_set>
#include "utility/graph.h"
#include "utility/grid.h"
#include "utility/geometry.h"
#include "config.h"

/**
 * @brief The (local) fundament describes the relevant surroundings of the spine head.
 *
 * It is inherently limited to coordinates reachable from some point on the spine
 * within two steps.
 *
 * The representation uses a bitmask in which the bit number <tt>n = (sly+x+2)*5 + (x+2)</tt>
 * is set to @c true if the grid location <tt>(x,sly): (sly+x) &#8712; [-2,2], x &#8712; [-2,2]</tt>
 * relative to the spine head is blocked, @c false if it is free.
 */
struct Fundament
{
	std::bitset<25> mask;

	Fundament() noexcept;
	Fundament(const Fundament& rhs) noexcept;
	Fundament(const Grid& grid, Coord spineHead) noexcept;

	bool operator==(const Fundament& rhs) const noexcept;

	/**
	 * @brief Given the relative coordinate @c c, determine whether it is occupied.
	 */
	bool blocked(Coord c) const noexcept;

	// print to stdout
	[[maybe_unused]]
	void print() const;

};

/**
 * @brief The identifying components of a partial dynamic programming problem in
 * the context of solving a particular lobster.
 *
 * Signatures define the equivalence classes of our problems.
 */
struct Signature
{
	int depth; //!< number of disks embedded so far
	Fundament fundament; //!< bit-encoded blocked spaces
	Coord head; //!< relative branch head position

	bool operator==(const Signature& rhs) const noexcept;
};

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

	DynamicProblem(InputDisks& input, const Grid& solution, Coord spineHead, Coord branchHead);

public:

	std::vector<DynamicProblem> subproblems() const;

	void setSolution(const Grid& solution, Coord spineHead, Coord branchHead);

	const Grid& solution() const noexcept;
	Coord spineHead() const noexcept;
	Coord branchHead() const noexcept;
	int depth() const noexcept;

	/**
	 * @brief Calculate the signaturue of the problem.
	 *
	 * The signature consists of the depth, fundament and branch head.
	 * We consider the fundament in an equivalent configuration that is unique
	 * up to mirroring. If the fundament is not normal, mirror it.
	 */
	Signature signature() const noexcept;

private:

	Grid solution_; // disks embedded so far
	Coord spineHead_;
	Coord branchHead_;
	InputDisks* input_;
	int depth_;

};

/**
 * @brief This queue supports the ordered expansion of DynamicProblems
 * from a set of open problems.
 *
 * It identifies the next problem to expand based on a priority measure
 * (size of the problem).
 *
 * It eliminates redundancy by keeping at most one problem instance for
 * any problem equivalence class.
 */
class ProblemQueue
{

public:

	ProblemQueue();

	const DynamicProblem& top() const noexcept;
	void push(DynamicProblem problem);
	void pop();
	bool empty() const noexcept;

	/**
	 * @brief Calculate a fixed-size value from the given signature.
	 *
	 * Derive the value from the depth, fundament and relative branch head.
	 *
	 * If we can determine that two given problems are equivalently solvable
	 * (eliminating redundancies like a mirrored fundament), their signatures
	 * also have the same hash value.
	 */
	static std::size_t hash(const Signature& signature) noexcept;

	/**
	 * @brief Determine whether two given problems are equivalently solvable.
	 *
	 * Determine this by comparing the problems' signatures.
	 *
	 * Two problems are equivalent if for any solution to @c lhs, we can efficiently
	 * derive a solution to @c rhs. This allows us to consider problems as equivalent
	 * even under some transformations to the fundament like mirroring and reachability.
	 */
	static bool equivalent(const DynamicProblem& lhs, const DynamicProblem& rhs) noexcept;

private:

	// problems to be expanded
	using OrderFunction = bool(*)(const DynamicProblem& lhs, const DynamicProblem& rhs);
	std::priority_queue<DynamicProblem, std::deque<DynamicProblem>, OrderFunction> open_;

	// set of hashes of already seen problems
	std::unordered_set<Signature, decltype(&hash)> closed_;

};
