#include "svg.h"
#include <algorithm>
#include <tuple>

Svg::Svg(const DiskGraph& graph, std::ostream& stream) noexcept :
	graph_(&graph), scale_(100.f), translate_(graph, 10.f, scale_), stream_(&stream)
{
}

void Svg::write()
{
	*stream_ <<
		"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
		"<svg xmlns=\"http://www.w3.org/2000/svg\">\n"
		"<g text-anchor=\"middle\">\n";

	auto writeAllDisks = [this](const std::vector<Disk>& v, Appearance a) {
		std::for_each(v.begin(), v.end(), [this, a](const Disk& d) { writeDisk(d, a); });
	};

	writeAllDisks(graph_->spines(), Appearance::SPINE);
	writeAllDisks(graph_->branches(), Appearance::BRANCH);
	writeAllDisks(graph_->leaves(), Appearance::LEAF);

	*stream_ << "</g></svg>\n";

	if (stream_->bad())
		throw std::exception("Failed to write SVG.");
}

void Svg::writeDisk(const Disk& disk, Appearance appearance)
{
	if (disk.failure)
		appearance = Appearance::FAIL;

	writeCircle(disk.x, disk.y, disk.id, appearance);

	if (const auto* parent = graph_->findDisk(disk.parent)) {
		writeLine(disk.x, disk.y, parent->x, parent->y);
	}
}

void Svg::writeCircle(float x, float y, int id, Appearance appearance)
{
	const Vec2 center = translate_.translate({ x, y });

	const char* fill;
	switch (appearance) {
	case Appearance::SPINE:
		fill = "beige";
		break;
	case Appearance::BRANCH:
		fill = "CadetBlue";
		break;
	case Appearance::LEAF:
		fill = "white";
		break;
	case Appearance::FAIL:
	default:
		fill = "crimson";
	}

	*stream_ << "\t<circle cx=\"" << center.x << "\" cy=\"" << center.y << "\" "
		<< "r=\"" << scale_ / 2 << "\" stroke=\"black\" stroke-width=\"3\" "
		<< "fill=\"" << fill << "\" /> "
		<< "<text x=\"" << center.x << "\" y=\"" << center.y + 6 << "\" font-size=\"16\">"
		<< id << "</text>\n";
}

void Svg::writeLine(float x1, float y1, float x2, float y2)
{
	constexpr float buffer = 0.15f; // distance between circle center and line end

	float sx = (x1 + buffer * (x2 - x1));
	float sy = (y1 + buffer * (y2 - y1));
	float tx = (x1 + (1 - buffer) * (x2 - x1));
	float ty = (y1 + (1 - buffer) * (y2 - y1));

	const Vec2 source = translate_.translate({ sx, sy });
	const Vec2 target = translate_.translate({ tx, ty });

	*stream_ << "\t<g stroke=\"black\" stroke-width=\"2\">"
		<< "<line x1=\"" << source.x << "\" y1=\"" << source.y << "\" x2=\""
		<< target.x << "\" y2=\"" << target.y << "\" /></g>\n";
}
