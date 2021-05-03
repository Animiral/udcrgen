#include "svg.h"
#include <fstream>
#include <algorithm>
#include <tuple>

/**
 * Describes how the vertex should be presented to the user.
 */
enum class Appearance { SPINE, LEAF, FAIL };

void write_circle(float x, float y, float scale, float offsetX, float offsetY,
	int id, Appearance appearance, std::ostream& stream);

void write_line(float x1, float y1, float x2, float y2, float scale, float offsetX, float offsetY,
	std::ostream& stream);

std::pair<float, float> calculate_offset(const DiskGraph& graph, float scale, float padding);

void write_svg(const DiskGraph& udcrg, const char* filename)
{
	const float padding = 10.f; // width of whitespace around the image
	const float scale = 100.f; // size of a unit disk in output

	// determine offset to move all circles on screen
	float offsetX, offsetY;
	std::tie(offsetX, offsetY) = calculate_offset(udcrg, scale, padding);

	std::ofstream stream{ filename };

	stream <<
		"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
		"<svg xmlns=\"http://www.w3.org/2000/svg\">\n"
		"<g text-anchor=\"middle\">\n";

	float prevX = 0, prevY = 0; // previous spine coordinates

	for (const auto& vertex : udcrg.spines()) {
		const Appearance appearance = vertex.failure ? Appearance::FAIL : Appearance::SPINE;

		write_circle(vertex.x, vertex.y, scale, offsetX, offsetY,
			vertex.id, appearance, stream);

		// connect spines to their predecessors
		if (prevX != 0) {
			write_line(vertex.x, vertex.y, prevX, prevY,
				scale, offsetX, offsetY, stream);
		}

		std::tie(prevX, prevY) = std::tie(vertex.x, vertex.y);
	}

	// TODO: branches

	for (const auto& vertex : udcrg.leaves()) {
		const Appearance appearance = vertex.failure ? Appearance::FAIL : Appearance::LEAF;

		write_circle(vertex.x, vertex.y, scale, offsetX, offsetY,
			vertex.id, appearance, stream);

		const auto& parent = udcrg.findDisk(vertex.parent);
		write_line(vertex.x, vertex.y, parent.x, parent.y,
			scale, offsetX, offsetY, stream);
	}

	stream << "</g></svg>\n";
	stream.close();

	if (stream.bad()) {
		throw std::exception((std::string("Failed to write SVG file: ") + filename).c_str());
	}
}

void write_circle(float x, float y, float scale, float offsetX, float offsetY,
	int id, Appearance appearance, std::ostream& stream)
{
	int cx = x * scale + offsetX;
	int cy = y * scale + offsetY;

	const char* fill;
	switch (appearance) {
	case Appearance::SPINE:
		fill = "beige";
		break;
	case Appearance::LEAF:
		fill = "white";
		break;
	case Appearance::FAIL:
	default:
		fill = "crimson";
	}

	stream << "\t<circle cx=\"" << cx << "\" cy=\"" << cy << "\" "
		<< "r=\"" << scale / 2 << "\" stroke=\"black\" stroke-width=\"3\" "
		<< "fill=\"" << fill << "\" /> "
		<< "<text x=\"" << cx << "\" y=\"" << cy+6 << "\" font-size=\"16\">"
		<< id << "</text>\n";
}

void write_line(float x1, float y1, float x2, float y2, float scale, float offsetX, float offsetY,
	std::ostream& stream)
{
	constexpr float margin = 0.15f;

	float sx = (x1 + margin * (x2 - x1)) * scale + offsetX;
	float sy = (y1 + margin * (y2 - y1)) * scale + offsetY;
	float tx = (x1 + (1 - margin) * (x2 - x1)) * scale + offsetX;
	float ty = (y1 + (1 - margin) * (y2 - y1)) * scale + offsetY;
	stream << "\t<g stroke=\"black\" stroke-width=\"2\">"
		<< "<line x1=\"" << sx << "\" y1=\"" << sy << "\" x2=\""
		<< tx << "\" y2=\"" << ty << "\" /></g>\n";
}

std::pair<float, float> calculate_offset(const DiskGraph& graph, float scale, float padding)
{
	const auto lessX = [](Disk a, Disk b) { return a.x < b.x; };

	const Disk& minxSpine = *std::min_element(graph.spines().begin(), graph.spines().end(), lessX);
	const Disk& minxBranch = *std::min_element(graph.branches().begin(), graph.branches().end(), lessX);
	const Disk& minxLeaf = *std::min_element(graph.leaves().begin(), graph.leaves().end(), lessX);
	const Disk& minxDisk = std::min({ minxSpine, minxBranch, minxLeaf }, lessX);

	const auto lessY = [](Disk a, Disk b) { return a.y < b.y; };

	const Disk& minySpine = *std::min_element(graph.spines().begin(), graph.spines().end(), lessY);
	const Disk& minyBranch = *std::min_element(graph.branches().begin(), graph.branches().end(), lessY);
	const Disk& minyLeaf = *std::min_element(graph.leaves().begin(), graph.leaves().end(), lessY);
	const Disk& minyDisk = std::min({ minySpine, minyBranch, minyLeaf }, lessY);

	return { -minxDisk.x * scale + scale / 2 + padding, -minyDisk.y * scale + scale / 2 + padding };
}
