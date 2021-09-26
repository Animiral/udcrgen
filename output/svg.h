// Output routine for SVG files

#pragma once

#include "graph.h"
#include "translate.h"
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
	float scale_; //!< size of a unit disk in output
	const Translate translate_; //!< coordinate translator
	std::ostream* stream_; //!< output stream

	/**
	 * Describes how the vertex should be presented to the user.
	 */
	enum class Appearance { SPINE, BRANCH, LEAF, FAIL };

	void writeDisk(const Disk& disk, Appearance appearance);
	void writeCircle(float x, float y, int id, Appearance appearance);
	void writeLine(float x1, float y1, float x2, float y2);

};
