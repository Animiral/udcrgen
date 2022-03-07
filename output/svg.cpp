#include "svg.h"
#include <algorithm>
#include <tuple>

Svg::Svg(std::ostream& stream, float scale) noexcept :
	stream_(&stream), scale_(scale), translate_(scale)
{
}

void Svg::intro() const
{
	*stream_ <<
		"<html>\n"
		"\t<body>\n";
}

void Svg::outro() const
{
	*stream_ << "\t</body>\n</html>\n";
}

void Svg::write(const DiskGraph& graph, const std::string& label)
{
	translate_.setLimits(graph, 10.f);

	openSvg(label);

	auto writeAllDisks = [this, graph](const std::vector<Disk>& v, Appearance a) {
		std::for_each(v.begin(), v.end(), [this, graph, a](const Disk& d) { writeDisk(d, graph, a); });
	};

	writeAllDisks(graph.spines(), Appearance::SPINE);
	writeAllDisks(graph.branches(), Appearance::BRANCH);
	writeAllDisks(graph.leaves(), Appearance::LEAF);

	closeSvg();
}

void Svg::write(const Signature& signature, const std::string& label)
{
	translate_.setLimits(-4.5, 2.5, 4.5, -2.5, 10.f);

	openSvg(label);

	for (int x = -2; x <= 2; x++) {
		for (int sly = -x - 2; sly <= 2 - x; sly++) {
			if (signature.fundament.blocked({ x, sly })) {
				float vecX = x + sly *.5f;
				float vecY = sly * 0.86602540378443864676372317075294f;
				writeCircle(vecX, vecY, "", signature.head == Coord{x, sly} ? "orange" : "grey");
			}
		}
	}

	closeSvg();
}

void Svg::openSvg(const std::string& label) const
{
	*stream_ <<
		"<details>\n"
		"\t<summary><h7>" << label << "</h7></summary>\n"
		"<svg class=\"content\" style=\"max-width:" << translate_.width() << ";\" "
		"viewBox=\"0 0 " << translate_.width() << " " << translate_.height() << "\">\n"
		"<g text-anchor=\"middle\">\n";
}

void Svg::closeSvg() const
{
	*stream_ << "</g></svg></details>\n";

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
