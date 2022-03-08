#pragma once

#include <utility>
#include "utility/graph.h"
#include "utility/grid.h"
#include "utility/geometry.h"
#include "utility/stat.h"
#include "config.h"

/**
 * The graph classes that the algorithms in this reportoire can differentiate.
 */
enum class GraphClass { CATERPILLAR, LOBSTER, OTHER };

/**
 * Take the raw input graph in the form of an edge list and prepare it for processing by the
 * embedding algorithm.
 *
 * The most important aspect is that we recognize which of the vertices constitute the
 * spine of the graph.
 *
 * Reorder all graph vertices with spines in front.
 * Then place branches in the order of
 * the spine vertex that is their parent.
 * Place leaves on a branch immediately following the branch vertex.
 *
 * @return the prepared graph and a type classification.
 */
std::pair<DiskGraph, GraphClass> classify(EdgeList input);

/**
 * An Embedder can embed single disks on the 2D plane.
 */
class Embedder
{

public:

	/**
	 * Place the next disk.
	 */
	virtual void embed(Disk& disk) = 0;

	/**
	 * @brief Configure the graph object to be used by this embedder.
	 *
	 * The graph allows us to look up parents of disks.
	 *
	 * This function will reset all previous embedding information
	 * except what is already stored in the graph.
	 */
	virtual void setGraph(DiskGraph& graph) noexcept = 0;

};

/**
 * A WholesaleEmbedder embeds disks given in a preordered list.
 */
class WholesaleEmbedder
{

public:

	/**
	 * @brief Embed the given ordered disks in the plane.
	 *
	 * @return true if an embedding was found, false otherwise.
	 */
	virtual bool embed(std::vector<Disk>& disks) = 0;

};

/**
 * Apply an embedding to the graph using the given embedding strategy.
 *
 * @return statistics on the embedding operation
 */
Stat embed(DiskGraph& graph, Embedder& embedder, Configuration::Algorithm algorithm, Configuration::EmbedOrder embedOrder);

/**
 * Apply an embedding to the graph using the dynamic programming approach.
 *
 * @return statistics on the embedding operation
 */
Stat embedDynamic(DiskGraph& graph, WholesaleEmbedder& embedder);
