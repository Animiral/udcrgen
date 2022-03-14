// Output routine for SVG files

#pragma once

#include "utility/graph.h"
#include "translate.h"
#include "dynamic.h"
#include <string>
#include <fstream>
#include <utility>
#include <filesystem>

/**
 * @brief Render objects in SVG, enveloped in HTML.
 */
class Svg
{

public:

	/**
	 * @brief Construct the SVG renderer for writing to the given stream.
	 */
	explicit Svg(std::ostream& stream) noexcept;

	/**
	 * @brief Construct the SVG renderer for writing to the given stream.
	 */
	explicit Svg(const std::filesystem::path& path);

	/**
	 * @brief Write SVG data to the given file.
	 *
	 * In the case of batch output, the base path will used only for the
	 * first batch (number 0). Further batches use the modified file name
	 * <samp>batch_3.html</samp>, where <samp>batch.html</samp> is the file
	 * name given in the base path and <samp>3</samp> is the current batch
	 * number.
	 */
	void open(std::filesystem::path basePath);

	/**
	 * @brief Explicitly close this @c Svg 's own output stream.
	 */
	void close();

	/**
	 * @brief Write output to the given stream.
	 *
	 * This causes the @c Svg to ignore the batch size, as it will not be
	 * able to re-open its own stream.
	 */
	void use(std::ostream& stream);

	/**
	 * @brief Configure the number of objects in one output file.
	 *
	 * With a batch size of 0, write all outputs to one file.
	 */
	void setBatchSize(int batchSize) noexcept;

	/**
	 * @brief Configure the size of disk graphics in the output.
	 *
	 * This mostly affects small graphs. When viewed in a browser,
	 * the whole graphic will shrink to fit the display size if
	 * necessary.
	 */
	void setScale(float scale) noexcept;

	/**
	 * @brief If the current batch is complete, open a new file for the next one.
	 *
	 * This includes writing the @c outro to the old file and writing the
	 * @c intro to the new file.
	 * @c Svg does not automatically @c ensureBatch on @c write.
	 * Users must call this function at appropriate times.
	 */
	void ensureBatch();

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
	void write(const DiskGraph& graph, const std::string& label) const;

	/**
	 * @brief Write the given problem signature to the given stream.
	 */
	void write(const Signature& signature, const std::string& label) const;

private:

	std::ofstream fileStream_; // owned stream object (optional)
	std::ostream* stream_; // current output stream
	std::filesystem::path basePath_; // output file base path (optional, required for batch numbering)
	int batchSize_; // maximum number of objects to write to a single file (with owned stream)
	int batchNr_; // current batch (with owned stream)
	mutable int batchCount_; // current number of objects written to the current file (with owned stream)

	float scale_; // size of a unit disk in output
	mutable Translate translate_; // internal coordinate translation and scaling

	// Describes how the vertex should be presented to the user.
	enum class Appearance { SPINE, BRANCH, LEAF, FAIL };

	void openSvg(const std::string& label) const; // write SVG opening tag and info
	void closeSvg() const; // write SVG closing tag and info

	void writeDisk(const Disk& disk, const DiskGraph& graph, Appearance appearance) const;
	void writeCircle(float x, float y, std::string label, std::string fill) const;
	void writeCircle(float x, float y, int id, Appearance appearance) const;
	void writeLine(float x1, float y1, float x2, float y2) const;

};
