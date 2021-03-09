// Output routine for SVG files

#pragma once

#include "graph.h"

/**
 * Render the embedded graph as a SVG and write it to the output file.
 */
void write_svg(const UdcrGraph& udcrg, const char* filename);
