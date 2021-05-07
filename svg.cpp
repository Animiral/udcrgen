#include "svg.h"
#include <algorithm>
#include <tuple>

Svg::Svg(const DiskGraph& graph, std::ostream& stream) noexcept :
	graph_(&graph), stream_(&stream), scale_(100.f)
{
	const float padding = 10.f; // width of whitespace around the image
	std::tie(offsetX_, offsetY_) = calculateOffset(padding);
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

template<typename Iterator, typename Less>
Disk min_disk(Iterator begin, Iterator end, Less less)
{
	auto min_elem = std::min_element(begin, end, less);
	return min_elem == end ? Disk{0, 0, 0, 0} : *min_elem;
}

std::pair<float, float> Svg::calculateOffset(float padding) noexcept
{
	const auto lessX = [](Disk a, Disk b) { return a.x < b.x; };

	const Disk& minxSpine = min_disk(graph_->spines().begin(), graph_->spines().end(), lessX);
	const Disk& minxBranch = min_disk(graph_->branches().begin(), graph_->branches().end(), lessX);
	const Disk& minxLeaf = min_disk(graph_->leaves().begin(), graph_->leaves().end(), lessX);
	const Disk& minxDisk = std::min({ minxSpine, minxBranch, minxLeaf }, lessX);

	const auto lessY = [](Disk a, Disk b) { return a.y < b.y; };

	const Disk& minySpine = min_disk(graph_->spines().begin(), graph_->spines().end(), lessY);
	const Disk& minyBranch = min_disk(graph_->branches().begin(), graph_->branches().end(), lessY);
	const Disk& minyLeaf = min_disk(graph_->leaves().begin(), graph_->leaves().end(), lessY);
	const Disk& minyDisk = std::min({ minySpine, minyBranch, minyLeaf }, lessY);

	return { -minxDisk.x * scale_ + scale_ / 2 + padding, -minyDisk.y * scale_ + scale_ / 2 + padding };
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
	int cx = x * scale_ + offsetX_;
	int cy = y * scale_ + offsetY_;

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

	*stream_ << "\t<circle cx=\"" << cx << "\" cy=\"" << cy << "\" "
		<< "r=\"" << scale_ / 2 << "\" stroke=\"black\" stroke-width=\"3\" "
		<< "fill=\"" << fill << "\" /> "
		<< "<text x=\"" << cx << "\" y=\"" << cy+6 << "\" font-size=\"16\">"
		<< id << "</text>\n";
}

void Svg::writeLine(float x1, float y1, float x2, float y2)
{
	constexpr float margin = 0.15f;

	float sx = (x1 + margin * (x2 - x1)) * scale_ + offsetX_;
	float sy = (y1 + margin * (y2 - y1)) * scale_ + offsetY_;
	float tx = (x1 + (1 - margin) * (x2 - x1)) * scale_ + offsetX_;
	float ty = (y1 + (1 - margin) * (y2 - y1)) * scale_ + offsetY_;
	*stream_ << "\t<g stroke=\"black\" stroke-width=\"2\">"
		<< "<line x1=\"" << sx << "\" y1=\"" << sy << "\" x2=\""
		<< tx << "\" y2=\"" << ty << "\" /></g>\n";
}
