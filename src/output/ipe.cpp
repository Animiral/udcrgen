#include "ipe.h"
#include "utility/exception.h"
#include <iomanip>
#include <cstring>

Ipe::Ipe(const DiskGraph& graph, std::ostream& stream) noexcept :
	graph_(&graph), scale_(16.f), translate_(scale_), stream_(&stream)
{
	translate_.setLimits(graph, 1.f);
}

void Ipe::write()
{
	*stream_ <<
		"<?xml version=\"1.0\"?>\n"
		"<!DOCTYPE ipe SYSTEM \"ipe.dtd\">\n"
		"<ipe version=\"70218\" creator=\"Ipe 7.2.23\">\n"
		"<info created=\"D:20210427103907\" modified=\"D:20210427105114\"/>\n"
		"<ipestyle name=\"min\">\n"
		"<symbol name=\"mark / hex_hor(sx)\" transformations=\"translations\">\n"
		"<path fill=\"sym - stroke\">\n"
		"-0.3 -0.52 m\n"
		"-0.6 0 l\n"
		"-0.3 0.52 l\n"
		"0.3 0.52 l\n"
		"0.6 0 l\n"
		"0.3 -0.52 l\n"
		"h\n"
		"</path>\n"
		"</symbol>\n"
		"<color name=\"KITblack\" value=\"0\"/>\n"
		"<color name=\"KITblack50\" value=\"0.5\"/>\n"
		"<color name=\"KITblue\" value=\"0.274 0.392 0.666\"/>\n"
		"<color name=\"KITblue50\" value=\"0.637 0.696 0.833\"/>\n"
		"<color name=\"KITgreen\" value=\"0 0.588 0.509\"/>\n"
		"</ipestyle>\n";

	// page start
	const char* layer = "alpha";
	*stream_ << "<page>\n<layer name=\"" << layer << "\"/>\n<view layers=\"" << layer << "\" active=\"" << layer << "\"/>\n";

	for (const Disk& disk : graph_->disks())
		writeDisk(disk);

	// page end
	*stream_ << "</page>";
	*stream_ << "</ipe>\n";

	if (stream_->fail())
		throw OutputException(std::strerror(errno));
}

void Ipe::writeDisk(const Disk& disk)
{
	assert(disk.depth >= 0);
	assert(disk.depth < 3);

	Appearance appearances[] = { Appearance::SPINE, Appearance::BRANCH, Appearance::LEAF };
	Appearance appearance = appearances[disk.depth];

	if (disk.failure)
		appearance = Appearance::FAIL;

	writeCircle(disk.x, disk.y, disk.id, appearance);

	if (const Disk* parent = disk.parent) {
		writeLine(disk.x, disk.y, parent->x, parent->y);
	}
}

void Ipe::writeCircle(float x, float y, int id, Appearance appearance)
{
	const Vec2 center = translate_.translate({ x, y });

	const char* ipe_color;
	switch (appearance) {
	case Appearance::SPINE:
		ipe_color = "KITblack50";
		break;
	case Appearance::BRANCH:
		ipe_color = "KITblue";
		break;
	case Appearance::LEAF:
		ipe_color = "KITgreen";
		break;
	case Appearance::FAIL:
	default:
		ipe_color = "KITblack";
	}

	*stream_ << "<path fill=\"" << ipe_color << "\">\n" << (scale_ / 2) << " 0 0 "
		<< (scale_ / 2) << " " << std::fixed << std::setprecision(3) << center.x << " " << center.y << " e\n</path>\n";
}

void Ipe::writeLine(float x1, float y1, float x2, float y2)
{
	const Vec2 source = translate_.translate({ x1, y1 });
	const Vec2 target = translate_.translate({ x2, y2 });

	*stream_ << "<path stroke=\"black\" pen=\"0.4\">\n" << source.x << " " << source.y
		<< " m\n" << target.x << " " << target.y << " l\n</path>\n";
}
