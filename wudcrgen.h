#pragma once

#include <utility>
#include "graph.h"

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
std::pair<DiskGraph, GraphClass> classify(const EdgeList& input);

/**
 * Generate a unit disk contact graph by computing the
 * appropriate embedding for the given caterpillar graph.
 *
 * This algorithm is described in:
 *   Boris Klemz, Martin Nöllenburg, and Roman Prutkin. Recognizing weighted disk contact
 *   graphs. In Emilio Di Giacomo and Anna Lubiw, editors, Graph Drawing and Network
 *   Visualization - 23rd International Symposium, GD 2015, Los Angeles, CA, USA, September
 *   24-26, 2015, Revised Selected Papers, volume 9411 of Lecture Notes in Computer Science,
 *   pages 433–446. Springer, 2015. doi:10.1007/978-3-319-27261-0_36.
 *
 * @param caterpillar: input caterpillar graph in degree representation
 * @param gap: minimum distance between two disks not in contact
 */
DiskGraph udcrgen(const Caterpillar& caterpillar, float gap);

/**
 * Generate a weak unit disk contact graph by computing the
 * appropriate embedding for the given caterpillar graph.
 *
 * This algorithm is described in:
 *   Jonas Cleve. Weak Unit Disk Contact Representations for
 *   Graphs without Embedding.
 *   36th European Workshop on Computational Geometry, Würzburg, Germany, March 16–18, 2020.
 */
DiskGraph wudcrgen(const Caterpillar& caterpillar);
