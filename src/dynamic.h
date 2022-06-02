// Embedding algorithms based on dynamic programming

#pragma once

#include <bitset>
#include <queue>
#include <set>
#include <memory>
#include "utility/grid.h"
#include "utility/geometry.h"
#include "utility/graph.h"
#include "embed.h"

/**
 * @brief The fundament describes the relevant surroundings of the spine head.
 *
 * It is inherently limited to coordinates reachable from some point on the spine
 * within two steps. These are called <em>local coordinates</em> and their value
 * is specified relative to the spine head; i.e. <tt>{0, 0}</tt> is the exact
 * location of the spine head.
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
	 * Return the index of the mask bit that reflects the blocked
	 * status of the given local coordinate, or -1 if the coordinate
	 * is not represented the fundament.
	 */
	static int index(Coord c) noexcept;

	/**
	 * Return the coordinate represented at the given bit index in the mask.
	 */
	static Coord at(int bit);

	/**
	 * @brief Given the relative coordinate @c c, determine whether it is occupied.
	 */
	bool blocked(Coord c) const noexcept;

	/**
	 * @brief Set the mask at the relative coordinate @c c to occupied.
	 */
	void block(Coord c) noexcept;

	/**
	 * @brief Change the center offset of the fundament.
	 *
	 * All blocked spaces in the fundament shift to represent the surroundings
	 * of the new {0, 0} center, which is moved one step in the given @c dir
	 * relative to the previous center.
	 */
	void shift(Dir dir) noexcept;

	/**
	 * Return a Fundament in which the non-blocked coordinates are exactly the
	 * ones which are reachable in this Fundament, from the given
	 * local start point, in the given number of steps.
	 */
	Fundament reachable(Coord from, int steps) const noexcept;

	/**
	 * Return the same result as @c reachable, but using only one "spine step",
	 * i.e. in directions which adhere to x-monotonocity.
	 */
	Fundament reachableBySpine(Coord from) const noexcept;

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

	/**
	 * Return @c true if this signature is preferable to the given signature
	 * with respect to chances of finding an embedding.
	 *
	 * This is the case if @c depth and @c head are equal and every space
	 * blocked in this fundament is also blocked in the other fundament.
	 *
	 * This definition is reflexive, i.e. every Signature dominates itself.
	 */
	bool dominates(const Signature& rhs) const noexcept;
};

using InputDisks = std::vector<Disk>; // complete input

/**
 * An instance of the dynamic programming problem.
 *
 * It consists of the immediate surroundings based on the solution this far
 * and an iterator into the ordered set of disks yet to solve.
 */
class DynamicProblem
{

public:

	/**
	 * @brief Create the root problem of the graph instance.
	 */
	explicit DynamicProblem(DiskGraph& graph);

private:

	/**
	 * @brief Create the child problem of the given problem.
	 *
	 * The next disk is to be placed in the given direction from the
	 * appropriate head (spine head or branch head).
	 *
	 * All children of the parent share the parent pointer because the
	 * original parent usually does not outlive the children.
	 */
	DynamicProblem(std::shared_ptr<const DynamicProblem> parent, Dir dir);

public:

	/**
	 * @brief Return the possible successor problems.
	 *
	 * Construct the successors by placing the next disk in order at one of
	 * the applicable free spaces.
	 */
	std::vector<DynamicProblem> subproblems() const;

	/**
	 * @brief Freely configure the object for testing.
	 */
	void setState(Fundament fundament, GraphTraversal position, Coord spineHead, Coord branchHead, int depth);

	/**
	 * @brief Return the current surroundings relevant to placement options.
	 */
	const Fundament& fundament() const noexcept;

	/**
	 * @brief Construct the full graph embedding from this and all parent embedded nodes.
	 */
	Grid solution() const;

	/**
	 * @brief Return the coordinate of the last placed spine.
	 *
	 * This is where we attach branches and the next spine disk.
	 */
	Coord spineHead() const noexcept;

	/**
	 * @brief Return the coordinate of the last placed branch.
	 *
	 * This is where we attach leaf disks.
	 */
	Coord branchHead() const noexcept;

	/**
	 * @brief Return the number of disks placed so far (0 at the root problem).
	 */
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

	Fundament fundament_; // spaces blocked by disks embedded so far
	Coord spineHead_;
	Coord branchHead_;
	GraphTraversal position_;
	GraphTraversal end_; // TODO: eliminate
	int depth_;
	std::shared_ptr<const DynamicProblem> parent_; // problem that this problem was derived from (by placing another disk). TODO: store only embedding info
	Coord placement_; // coord of last disk placed

};

/**
 * @brief Determine the normalized base fundament with regards to reachability.
 *
 * Starting from the given @c base fundament with the given local @c head,
 * while embedding the given @c input disks at the given @c depth,
 * Consider all possible extension layouts and mark all locations as "blocked"
 * which are irrelevant due to being out of reach for the remaining embedding
 * problem.
 *
 * @param base start from this fundament
 * @param head local branch head coordinate
 * @param input list of input disks to embed
 * @param depth current problem depth
 * @return a fundament like @c base, but all unreachable coords are blocked
 */
Fundament reachableEventually(Fundament base, Coord head, GraphTraversal position, GraphTraversal end) noexcept;

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
	void push(const DynamicProblem& problem);
	void pop();
	bool empty() const noexcept;

	/**
	 * Define a weak ordering on Signatures such that Signatures of the same
	 * size and same head are grouped next to each other, and Signatures
	 * with smaller Fundaments (less blocked spaces) are less than Signatures
	 * with larger Fundaments.
	 *
	 * This allows us to quickly find out if the "dominance" heuristic
	 * applies to a problem to be pushed.
	 */
	static bool less(const Signature& lhs, const Signature& rhs) noexcept;

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
	std::set<Signature, decltype(&less)> closed_;

};

/**
 * This embedder feeds the disks into a dynamic programming problem
 * and operates the queue to produce the embedding result.
 */
class DynamicProblemEmbedder : public WholesaleEmbedder
{

public:

	virtual bool embed(DiskGraph& graph) override;

};
