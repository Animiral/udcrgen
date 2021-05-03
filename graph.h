// Custom graph representations

#pragma once

#include <vector>
#include <istream>

/**
 * A basic representation of a caterpillar graph used for input.
 *
 * The caterpillar graph is a string of leaf counts, i.e. for every
 * vertex in the spine, it stores the number of leaves attached,
 * which is the degree of the vertex minus neighbor spine vertices.
 */
class Caterpillar
{

public:

	/**
	 * Append another spine vertex to the end of the
	 * spine with the given number of leaves.
	 */
	void extend(int leaves);

	/**
	 * Count the total number of vertices (spine + leaves).
	 */
	int countVertices() const noexcept;

	/**
	 * Get the number of vertices in the spine.
	 */
	int countSpine() const noexcept;

	/**
	 * Get the list of leaf counts.
	 */
	const std::vector<int>& leaves() const noexcept;

	/**
	 * Parse a text representation from the given stream.
	 */
	static Caterpillar fromText(std::istream& stream);

private:

	std::vector<int> leaves_;

};

/**
 * Represents one edge in a yet-unrecognized graph.
 *
 * We use a list of edges as an intermediary representation from reading
 * the graph from any input format. Afterwards, it needs to be classified
 * before being processed into an embedding.
 */
struct Edge
{
	int from; //!< start vertex number
	int to; //!< end vertex number
};

using EdgeList = std::vector<Edge>;

/**
 * Parse a text representation of an edge list from the given stream.
 *
 * We expect the text to consist of lines in the format
 * <from> <to>
 * where <from> is the id of a some vertex and <to> is the id
 * of another vertex which it connects to.
 */
EdgeList edgesFromText(std::istream& stream);

/**
 * Reorder the edge list from begin to end.
 * Move edges which connect leaves to the back and others to the front.
 * Turn all leaf-adjacent edges to always point *to* the leaf.
 *
 * @return the new past-the-end iterator that indicates the end of non-leaf edges.
 */
EdgeList::iterator separateLeaves(EdgeList::iterator begin, EdgeList::iterator end);

/**
 * Determine whether the given edge list describes a path - a series of vertices
 * connected in a row. Only a path can be the spine of a caterpillar or lobster.
 *
 * If the edges do indeed describe a path, re-order them in the order in which
 * they can be traversed from beginning to end.
 *
 * @return true if the edges describe a path, false otherwise.
 */
bool recognizePath(EdgeList::iterator begin, EdgeList::iterator end);

/**
 * A single unit-sized disk for the output graph representation.
 *
 * It has a unique vertex number within the graph and
 * 2D coordinates to represent the embedding.
 */
struct Disk
{
	int id; //!< unique vertex number [0..n]
	int parent; //!< parent vertex number, must be spine vertex or -1
	float x, y; //!< embedding coordinates
	bool failure; //!< whether the algorithm failed to place this vertex in UDCR
};

/**
 * The output graph representation.
 *
 * It consists of a list of disks, in which the spine disks
 * can be recognized by the convention of being placed at the front
 * of the list.
 */
class DiskGraph
{

public:

	/**
	 * Construct the graph.
	 *
	 * @param disks: number of total disks
	 * @param spine: length of the central disk chain
	 */
	DiskGraph(int disks, int spine);

	/**
	 * Get the length of the spine a.k.a. backbone.
	 *
	 * All disks at index 0 to spine() - 1 are spine disks.
	 */
	int spine() const noexcept;

	/**
	 * Get the mutable vertex data.
	 */
	std::vector<Disk>& disks() noexcept;

	/**
	 * Get the immutable vertex data.
	 */
	const std::vector<Disk>& disks() const noexcept;

	/**
	 * Get the vertex with the given unique number.
	 *
	 * Throw an exception if the vertex does not exist.
	 */
	const Disk& findDisk(int id) const;

	/**
	 * Create an instance based on the given basic caterpillar representation.
	 *
	 * The embedding coordinates of the resulting disks are unspecified.
	 */
	static DiskGraph fromCaterpillar(const Caterpillar& caterpillar);

private:

	std::vector<Disk> disks_;
	int spine_;

};
