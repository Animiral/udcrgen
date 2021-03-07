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
 * A single vertex for the output graph representation.
 *
 * It has a unique number within the graph and
 * 2D coordinates to represent the embedding.
 *
 * It also knows information for ordering relative to other vertices,
 * which is used for assigning the embedding position.
 */
struct UdcrVertex
{
	int id; //!< unique vertex number [0..n]
	int parent; //!< parent vertex number, must be part of the spine in caterpillar
	int rank; //!< child number relative to the parent, >= 0
	float x, y; //!< embedding coordinates
};

/**
 * The output graph representation.
 *
 * It consists of a list of vertices, in which the spine vertices
 * can be recognized by the convention of being placed at the front
 * of the list.
 */
class UdcrGraph
{

public:

	/**
	 * Construct the graph.
	 *
	 * @param vertices: number of total vertices
	 * @param spine: length of the central vertex chain
	 */
	UdcrGraph(int vertices, int spine);

	/**
	 * Get the length of the spine a.k.a. backbone.
	 *
	 * All vertices at index 0 to spine() - 1 are spine vertices.
	 */
	int spine() const noexcept;

	/**
	 * Get the mutable vertex data.
	 */
	std::vector<UdcrVertex>& vertices() noexcept;

	/**
	 * Get the immutable vertex data.
	 */
	const std::vector<UdcrVertex>& vertices() const noexcept;

private:

	std::vector<UdcrVertex> vertices_;
	int spine_;

};
