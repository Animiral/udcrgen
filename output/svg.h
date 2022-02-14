// Output routine for SVG files

#pragma once

#include "utility/graph.h"
#include "translate.h"
#include "dynamic.h"
#include <fstream>
#include <utility>

/**
 * @brief Render objects in SVG.
 */
class Svg
{

public:

	/**
	 * @brief Construct the SVG renderer.
	 */
	explicit Svg(float scale = 100) noexcept;

	/**
	 * @brief Write the given graph to the given stream.
	 */
	void write(const DiskGraph& graph, std::ostream& stream);

	/**
	 * @brief Write the given problem signature to the given stream.
	 */
	void write(const Signature& signature, std::ostream& stream);

private:

	float scale_; //!< size of a unit disk in output
	Translate translate_; //!< coordinate translation and scaling
	std::ostream* stream_; //!< current output stream

	/**
	 * Describes how the vertex should be presented to the user.
	 */
	enum class Appearance { SPINE, BRANCH, LEAF, FAIL };

	void writeIntro() const;
	void writeOutro() const;

	void writeDisk(const Disk& disk, const DiskGraph& graph, Appearance appearance) const;
	void writeCircle(float x, float y, std::string label, std::string fill) const;
	void writeCircle(float x, float y, int id, Appearance appearance) const;
	void writeLine(float x1, float y1, float x2, float y2) const;

};
