// An algorithm for batch testing auto-generated lobster instances of incremental size

#pragma once

#include "utility/graph.h"
#include "embed.h"
#include "output/svg.h"
#include "config.h"
#include <vector>
#include <chrono>

/**
 * Record of one execution of an embedding algorithm.
 */
struct Stat
{
	Configuration::Algorithm algorithm;
	int size; //!< total number of input vertices
	int spines; //!< number of spine input vertices

	bool success; //!< true if an embedding was determined possible, false otherwise
	std::chrono::milliseconds duration; //!< run duration of algorithm
};

/**
 * This enumerator implements an iterator pattern to generate a specified
 * range of Lobster instances.
 *
 * It offers tools to examine the generated instances by using the embedding
 * algorithms that are available.
 *
 * It collects statistics on feasibility of unit disk embeddings and performance,
 * comparing a fast heuristic implementation with a reliable dynamic programming
 * implementation.
 */
class Enumerate
{

public:

	Enumerate(Embedder& fast, WholesaleEmbedder& reference, int minSize, int maxSize) noexcept;

	/**
	 * @brief Advance to the next lobster in the enumeration.
	 *
	 * This may involve shifting leaves between branches, adding leaves, branches,
	 * and spines with no upper limit on the size of the result.
	 *
	 * Before advancing to the next instance, the current instance must have been tested.
	 */
	void next();

	/**
	 * Run the embedding algorithms on the current lobster and record the results
	 * in the statistics.
	 */
	void test();

	/**
	 * Run the embedding algorithms on the given lobster and record the results
	 * in the statistics.
	 */
	void test(const Lobster& lobster);

	/**
	 * Get the current Lobster instance.
	 */
	const Lobster& current() const noexcept;

	/**
	 * Set the @c EmbedOrder for the fast embedder.
	 */
	void setEmbedOrder(Configuration::EmbedOrder embedOrder) noexcept;

	/**
	 * @brief Configure the output handler.
	 *
	 * If set, in addition to collecting statistics, write a witness for every
	 * result, if available.
	 */
	void setOutput(Svg& output);

	/**
	 * Access statistics gathered.
	 */
	const std::vector<Stat>& stats() const noexcept;

	/**
	 * @brief Convenience function to execute a batch of tests.
	 *
	 * Equivalent to calling @c test() and @c next() until all instances with at
	 * least @c minSize spine vertices and less than @c maxSize spine vertices
	 * have been evaluated and entered into the @c stats.
	 */
	void run();

private:

	Embedder* fast_;
	Configuration::EmbedOrder embedOrder_;
	WholesaleEmbedder* reference_;
	int minSize_;
	int maxSize_;
	Lobster current_;
	Svg* output_;
	std::vector<Stat> stats_;

};

