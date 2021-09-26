// Shared routines for translating graph layout grid coordinates to target canvas coordinates

#pragma once

#include "graph.h"
#include "geometry.h"

/**
 * Translate coordinates of disks in the graph to a target canvas.
 *
 * The graph coordinates from the layout heuristic usually put
 * the first unit disk of the spine at the origin (0/0) and
 * use a disk diameter of 1.0.
 * 
 * The output canvas in formats like SVG and IPE uses positive
 * coordinates only and requires a blank margin from the axis
 * to look nice.
 */
class Translate
{

public:

	/**
	 * Construct the translator for the given graph, margin and scale.
	 */
	explicit Translate(const DiskGraph& graph, float margin, float scale) noexcept;

	/**
	 * Return the given layout coordinates translated into canvas coordinates.
	 */
	Vec2 translate(Vec2 v) const noexcept;

private:

	// furthest object extents in graph layout units
	float left_, right_, top_, bottom_;

	float margin_; //!< blank distance from canvas border
	float scale_; //!< size of a unit disk in canvas units

};
