#include "svg.h"
#include <fstream>
#include <algorithm>

void write_circle(float x, float y, float scale, float offsetX, float offsetY,
	int id, bool isSpine, std::ostream& stream);

void write_line(float x1, float y1, float x2, float y2, float scale, float offsetX, float offsetY,
	std::ostream& stream);

void write_svg(const UdcrGraph& udcrg, const char* filename)
{
	const float scale = 100.f; // size of a unit disk in output

	// determine offset to move all circles on screen
	const float offsetX = -std::min_element(udcrg.vertices().begin(), udcrg.vertices().end(),
		[](UdcrVertex a, UdcrVertex b) { return a.x < b.x; })->x * scale + scale / 2;
	const float offsetY = -std::min_element(udcrg.vertices().begin(), udcrg.vertices().end(),
		[](UdcrVertex a, UdcrVertex b) { return a.y < b.y; })->y * scale + scale / 2;

	std::ofstream stream{ filename };

	stream <<
		"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
		"<svg xmlns=\"http://www.w3.org/2000/svg\">\n"
		"<g text-anchor=\"middle\">\n";

	for (const auto& vertex : udcrg.vertices()) {
		write_circle(vertex.x, vertex.y, scale, offsetX, offsetY,
			vertex.id, vertex.id < udcrg.spine(), stream);

		if (vertex.parent >= 0) {
			const auto& parent = udcrg.findVertex(vertex.parent);
			write_line(vertex.x, vertex.y, parent.x, parent.y,
				scale, offsetX, offsetY, stream);
		}
	}

	stream << "</g></svg>\n";
	stream.close();

	if (stream.bad()) {
		throw std::exception((std::string("Failed to write SVG file: ") + filename).c_str());
	}
}

void write_circle(float x, float y, float scale, float offsetX, float offsetY,
	int id, bool isSpine, std::ostream& stream)
{
	int cx = x * scale + offsetX;
	int cy = y * scale + offsetY;
	const char* fill = isSpine ? "beige" : "white";
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
