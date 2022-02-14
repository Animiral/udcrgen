#include "svg.h"
#include <algorithm>
#include <tuple>

Svg::Svg(float scale) noexcept :
	scale_(scale), translate_(scale)
{
}

void Svg::write(const DiskGraph& graph, std::ostream& stream)
{
	translate_.setLimits(graph, 10.f);
	stream_ = &stream;

	writeIntro();

	auto writeAllDisks = [this, graph](const std::vector<Disk>& v, Appearance a) {
		std::for_each(v.begin(), v.end(), [this, graph, a](const Disk& d) { writeDisk(d, graph, a); });
	};

	writeAllDisks(graph.spines(), Appearance::SPINE);
	writeAllDisks(graph.branches(), Appearance::BRANCH);
	writeAllDisks(graph.leaves(), Appearance::LEAF);

	writeOutro();
}

void Svg::write(const Signature& signature, std::ostream& stream)
{
	translate_.setLimits(-4.5, 2.5, 4.5, -2.5, 10.f);
	stream_ = &stream;

	writeIntro();

	for (int x = -2; x <= 2; x++) {
		for (int sly = -x - 2; sly <= 2 - x; sly++) {
			if (signature.fundament.blocked({ x, sly })) {
				float vecX = x + sly *.5f;
				float vecY = sly * 0.86602540378443864676372317075294f;
				writeCircle(vecX, vecY, "", signature.head == Coord{x, sly} ? "orange" : "grey");
			}
		}
	}

	writeOutro();
}

void Svg::writeIntro() const
{
	*stream_ <<
		"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
		"<svg xmlns=\"http://www.w3.org/2000/svg\" "
		"viewBox=\"0 0 " << translate_.width() << " " << translate_.height() << "\">\n"
		"<g text-anchor=\"middle\">\n";
}

void Svg::writeOutro() const
{
	*stream_ << "</g></svg>\n";

	if (stream_->bad())
		throw std::exception("Failed to write SVG.");
}

void Svg::writeDisk(const Disk& disk, const DiskGraph& graph, Appearance appearance) const
{
	if (disk.failure)
		appearance = Appearance::FAIL;

	writeCircle(disk.x, disk.y, disk.id, appearance);

	if (const auto* parent = graph.findDisk(disk.parent)) {
		writeLine(disk.x, disk.y, parent->x, parent->y);
	}
}

void Svg::writeCircle(float x, float y, std::string label, std::string fill) const
{
	const Vec2 center = translate_.translate({ x, y });

	*stream_ << "\t<circle cx=\"" << center.x << "\" cy=\"" << center.y << "\" "
		<< "r=\"" << scale_ / 2 << "\" stroke=\"black\" stroke-width=\"3\" "
		<< "fill=\"" << fill << "\" /> "
		<< "<text x=\"" << center.x << "\" y=\"" << center.y + 6 << "\" font-size=\"16\">"
		<< label << "</text>\n";
}

void Svg::writeCircle(float x, float y, int id, Appearance appearance) const
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

	writeCircle(x, y, std::to_string(id), fill);
}

void Svg::writeLine(float x1, float y1, float x2, float y2) const
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
