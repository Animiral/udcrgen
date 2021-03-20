#pragma once

#include "graph.h"

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
UdcrGraph udcrgen(const Caterpillar& caterpillar, float gap);

/**
 * Generate a weak unit disk contact graph by computing the
 * appropriate embedding for the given caterpillar graph.
 *
 * This algorithm is described in:
 *   Jonas Cleve. Weak Unit Disk Contact Representations for
 *   Graphs without Embedding.
 *   36th European Workshop on Computational Geometry, Würzburg, Germany, March 16–18, 2020.
 */
UdcrGraph wudcrgen(const Caterpillar& caterpillar);
