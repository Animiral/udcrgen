#pragma once

#include <utility>
#include "utility/graph.h"
#include "utility/grid.h"
#include "utility/geometry.h"
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

};

/**
 * Apply an embedding to the graph using the given embedding strategy.
 */
void embed(DiskGraph& graph, Embedder& embedder, Configuration::EmbedOrder embedOrder);
