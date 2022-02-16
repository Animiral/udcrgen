// Output routine for SVG files

#pragma once

#include "utility/graph.h"
#include "translate.h"
#include "dynamic.h"
#include <string>
#include <fstream>
#include <utility>

/**
 * @brief Render objects in SVG, enveloped in HTML.
 */
class Svg
{

public:

	/**
	 * @brief Construct the SVG renderer for writing to the given stream.
	 */
	explicit Svg(std::ostream& stream, float scale = 100) noexcept;

	/**
	 * @brief Write the HTML header and introductory information.
	 *
	 * Must be called before writing contents.
	 */
	void intro() const;

	/**
	 * @brief Write the conclusion of the HTML document.
	 *
	 * Must be called after writing contents.
	 */
	void outro() const;

	/**
	 * @brief Write the given graph.
	 */
	void write(const DiskGraph& graph, const std::string& label);

	/**
	 * @brief Write the given problem signature to the given stream.
	 */
	void write(const Signature& signature, const std::string& label);

private:

	std::ostream* stream_; // current output stream
	float scale_; // size of a unit disk in output
	Translate translate_; // coordinate translation and scaling
	bool firstContent_; // true before any (non-intro) content has been written

	// Describes how the vertex should be presented to the user.
	enum class Appearance { SPINE, BRANCH, LEAF, FAIL };

	void openSvg(const std::string& label) const; // write SVG opening tag and info
	void closeSvg() const; // write SVG closing tag and info

	void writeDisk(const Disk& disk, const DiskGraph& graph, Appearance appearance) const;
	void writeCircle(float x, float y, std::string label, std::string fill) const;
	void writeCircle(float x, float y, int id, Appearance appearance) const;
	void writeLine(float x1, float y1, float x2, float y2) const;

};
