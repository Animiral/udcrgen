// Custom graph representations

#pragma once

#include <array>
#include <vector>
#include "geometry.h"
#include "config.h" // TODO: refactor

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
 * A basic representation of a lobster graph.
 *
 * The lobster graph is a string of spine vertices. For every spine vertex, it stores
 * up to five branches, to each of which up to five leaves may be attached.
 *
 * The representation of a branch is simply the number of attached leaves (0-5).
 * Spines are represented by a 5-element list of branches, in which excess
 * elements at the back are marked by a special NO_BRANCH value.
 */
class Lobster
{

public:

	using Spine = std::array<int, 5>; // single spine node
	static const int NO_BRANCH; // = -1

	Lobster() noexcept;
	Lobster(Lobster&& ) noexcept;
	Lobster(std::vector<Spine> spine) noexcept;
	Lobster& operator=(Lobster&& ) noexcept;

	bool operator==(const Lobster& rhs) const noexcept;

	/**
	 * Count the total number of vertices (spine + branches + leaves).
	 */
	int countVertices() const noexcept;

	/**
	 * Get the number of nodes in the spine.
	 */
	int countSpine() const noexcept;

	/**
	 * Read access to the underlying spine data.
	 */
	const std::vector<Spine>& spine() const noexcept;

	/**
	 * Full access to the underlying spine data.
	 */
	std::vector<Spine>& spine() noexcept;

private:

	std::vector<Spine> spine_;

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
 * @brief Allow traversal of the graph.
 *
 * In depth-first order, explore all leaves immediately after their parent branch.
 * In breadth-first order, explore branches before leaves on each spine.
 */
class GraphTraversal
{

public:

	/**
	 * Construct an end iterator.
	 */
	GraphTraversal() noexcept;

	/**
	 * Construct an iterator starting from the given disk.
	 */
	explicit GraphTraversal(Disk* from, Configuration::EmbedOrder order) noexcept;

	GraphTraversal& operator++() noexcept;
	Disk& operator*() const noexcept;
	Disk* operator->() const noexcept;
	bool operator==(const GraphTraversal& rhs) const noexcept;
	bool operator!=(const GraphTraversal& rhs) const noexcept;

private:

	Disk* disk_;
	Configuration::EmbedOrder order_;

};

/**
 * @brief The output graph representation.
 *
 * It stores of a list of disks and provides an interface to conveniently
 * traverse and manipulate them.
 *
 * While the disks are mutable, the graph itself is designed to be fixed-size.
 */
class DiskGraph
{

public:

	/**
	 * @brief Construct the graph from disk data.
	 *
	 * The tip pointer point to the first spine node.
	 * By default, assume that it is the first element in @c disks.
	 */
	explicit DiskGraph(std::vector<Disk>&& disks = {}, Disk* tip = nullptr);

	DiskGraph(const DiskGraph& rhs);
	DiskGraph(DiskGraph&& rhs) noexcept;
	DiskGraph& operator=(const DiskGraph& rhs);
	DiskGraph& operator=(DiskGraph&& rhs) noexcept;

	std::vector<Disk>& disks() noexcept;
	Disk* tip() noexcept;
	const std::vector<Disk>& disks() const noexcept;
	const Disk* tip() const noexcept;

	GraphTraversal traversal(Configuration::EmbedOrder order) noexcept;
	GraphTraversal end() const noexcept;

	/**
	 * Return the number of disks in the graph.
	 */
	std::size_t size() const noexcept;

	/**
	 * Return the number of spine disks in the graph.
	 */
	std::size_t length() const noexcept;

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
	 * @brief Create an instance based on the given basic caterpillar representation.
	 *
	 * The resulting graph is not embedded, thus the embedding coordinates of
	 * the disks and related fields are unspecified.
	 */
	static DiskGraph fromCaterpillar(const Caterpillar& caterpillar);

	/**
	 * @brief Create an instance based on the given basic lobster representation.
	 *
	 * The resulting graph is not embedded, thus the embedding coordinates of
	 * the disks and related fields are unspecified.
	 */
	static DiskGraph fromLobster(const Lobster& lobster);

	/**
	 * Return the edge list representation of this graph.
	 *
	 * Additional information, such as the vertex coordinates, is lost.
	 */
	EdgeList toEdgeList() const;

private:

	std::vector<Disk> disks_;
	Disk* tip_;

	void fixDiskPointer(const DiskGraph& base, Disk*& pointer) noexcept;

};
