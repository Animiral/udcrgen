// Output routine for SVG files

#pragma once

#include "graph.h"
#include <fstream>
#include <utility>

/**
 * Render a given graph in SVG.
 */
class Svg
{

public:

	/**
	 * Construct the SVG renderer to print the given graph to the given output stream.
	 */
	explicit Svg(const DiskGraph& graph, std::ostream& stream) noexcept;

	/**
	 * Perform the rendering.
	 */
	void write();

private:

	const DiskGraph* graph_; //!< output object
	std::ostream* stream_; //!< output stream
	float offsetX_, offsetY_; //!< offset to move all circles on screen
	float scale_; //!< size of a unit disk in output

	/**
	 * Describes how the vertex should be presented to the user.
	 */
	enum class Appearance { SPINE, BRANCH, LEAF, FAIL };

	std::pair<float, float> calculateOffset(float padding) noexcept;
	void writeDisk(const Disk& disk, Appearance appearance);
	void writeCircle(float x, float y, int id, Appearance appearance);
	void writeLine(float x1, float y1, float x2, float y2);

};
