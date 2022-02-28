// Shared routines for translating graph layout grid coordinates to target canvas coordinates

#pragma once

#include "utility/graph.h"
#include "utility/geometry.h"

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
	explicit Translate(float scale) noexcept;

	/**
	 * Return the given layout coordinates translated into canvas coordinates.
	 */
	Vec2 translate(Vec2 v) const noexcept;

	/**
	 * @brief Configure offsets to show all disks in the given range.
	 *
	 * @param top expected y-coordinate of top disk (smallest y)
	 * @param right expected x-coordinate of rightmost disk
	 * @param bottom expected y-coordinate of bottom disk (largest y)
	 * @param left expected x-coordinate of leftmost disk
	 * @param margin extra distance from x- and y-axes
	 */
	void setLimits(float top, float right, float bottom, float left, float margin) noexcept;

	/**
	 * @brief Configure offsets for the specific disks in the graph.
	 *
	 * Convenience function.
	 */
	void setLimits(const DiskGraph& graph, float margin) noexcept;

	float width() const noexcept;
	float height() const noexcept;

private:

	// furthest object extents in graph layout units
	float top_, right_, bottom_, left_;

	float margin_; //!< blank distance from canvas border
	float scale_; //!< size of a unit disk in canvas units

};
