#pragma once

#include "graph.h"

/**
 * Generate a weak unit disk contact graph by computing the
 * appropriate embedding for the given caterpillar graph.
 *
 * This algorithm is described in:
 *   Boris Klemz, Martin Nöllenburg, and Roman Prutkin. Recognizing weighted disk contact
 *   graphs. In Emilio Di Giacomo and Anna Lubiw, editors, Graph Drawing and Network
 *   Visualization - 23rd International Symposium, GD 2015, Los Angeles, CA, USA, September
 *   24-26, 2015, Revised Selected Papers, volume 9411 of Lecture Notes in Computer Science,
 *   pages 433–446. Springer, 2015. doi:10.1007/978-3-319-27261-0\_36.
 */
UdcrGraph wudcrgen(const Caterpillar& caterpillar);
