// Custom graph representations

#pragma once

#include <vector>
#include <istream>
#include <ostream>
#include "geometry.h"

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
	DiskId from; //!< start vertex number
	DiskId to; //!< end vertex number
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
EdgeList edges_from_text(std::istream& stream);

/**
 * Write a text representation of an edge list to the given stream.
 */
void edges_to_text(std::ostream& stream, const EdgeList& edges);

/**
 * Reorder the edge list from begin to end.
 * Move edges which connect leaves to the back and others to the front.
 * Turn all leaf-adjacent edges to always point *to* the leaf.
 *
 * @return the new past-the-end iterator that indicates the end of non-leaf edges.
 */
EdgeList::iterator separate_leaves(EdgeList::iterator begin, EdgeList::iterator end);

/**
 * Determine whether the given edge list describes a path - a series of vertices
 * connected in a row. Only a path can be the spine of a caterpillar or lobster.
 *
 * If the edges do indeed describe a path, re-order them in the order in which
 * they can be traversed from beginning to end.
 *
 * @return true if the edges describe a path, false otherwise.
 */
bool recognize_path(EdgeList::iterator begin, EdgeList::iterator end);

/**
 * The output graph representation.
 *
 * It consists of lists of disks, separated by their
 * role as spine, branch or leaf disks.
 */
class DiskGraph
{

public:

	/**
	 * Construct the graph.
	 *
	 * @param spines: length of the central disk chain
	 * @param branches: number of branch disks
	 * @param leaves: number of leaf disks
	 */
	DiskGraph(int spines, int branches, int leaves);

	/**
	 * Get the mutable spine vertex data.
	 */
	std::vector<Disk>& spines() noexcept;

	/**
	 * Get the immutable spine vertex data.
	 */
	const std::vector<Disk>& spines() const noexcept;

	/**
	 * Get the mutable branch vertex data.
	 */
	std::vector<Disk>& branches() noexcept;

	/**
	 * Get the immutable branch vertex data.
	 */
	const std::vector<Disk>& branches() const noexcept;

	/**
	 * Get the mutable leaf vertex data.
	 */
	std::vector<Disk>& leaves() noexcept;

	/**
	 * Get the immutable leaf vertex data.
	 */
	const std::vector<Disk>& leaves() const noexcept;

	/**
	 * Return the number of disks in the graph.
	 */
	std::size_t size() const noexcept;

	/**
	 * Get the disk with the given vertex id.
	 *
	 * @return a pointer to the disk or nullptr if the id is unknown.
	 */
	Disk* findDisk(DiskId id);

	/**
	 * Get the disk with the given vertex id.
	 *
	 * @return a pointer to the disk or nullptr if the id is unknown.
	 */
	const Disk* findDisk(DiskId id) const;

	/**
	 * Create an instance based on the given basic caterpillar representation.
	 *
	 * The embedding coordinates of the resulting disks are unspecified.
	 */
	static DiskGraph fromCaterpillar(const Caterpillar& caterpillar);

	/**
	 * Return the edge list representation of this graph.
	 *
	 * Additional information, such as the vertex coordinates, is lost.
	 */
	EdgeList toEdgeList() const;

private:

	std::vector<Disk> spines_;
	std::vector<Disk> branches_;
	std::vector<Disk> leaves_;

};
