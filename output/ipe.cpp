#include "ipe.h"
#include <algorithm>
#include <tuple>
#include <iomanip>

namespace
{
	// GRID TO COORDINATE TRANSLATION

	const float gstep = 16.f;
	const float gstep_s = (sqrtf(3.f) * gstep) / 2.f;

		//t_add = lambda i, j: i + j
		//def t_addition(a, b) :
		//return tuple(map(t_add, a, b))
	void t_addition(float& r1, float& r2, float a1, float a2)
	{
		r1 += a1;
		r2 += a2;
	}

	void grid_to_coord(float a, float b, float c, float& x, float& y)
	{
		// print("\n###")
		// print(str(a) + ", " + str(b) + ", " + str(c))
		x = y = 0;
		t_addition(x, y, a * gstep, 0);
		t_addition(x, y, b * gstep / 2, b * gstep_s);
		t_addition(x, y, -c * gstep / 2, c * gstep_s);
	}

}


Ipe::Ipe(const DiskGraph& graph, std::ostream& stream) noexcept :
	graph_(&graph), scale_(16.f), translate_(graph, 1.f, scale_), stream_(&stream)
{
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

	auto writeAllDisks = [this](const std::vector<Disk>& v, Appearance a) {
		std::for_each(v.begin(), v.end(), [this, a](const Disk& d) { writeDisk(d, a); });
	};

	writeAllDisks(graph_->spines(), Appearance::SPINE);
	writeAllDisks(graph_->branches(), Appearance::BRANCH);
	writeAllDisks(graph_->leaves(), Appearance::LEAF);

	// page end
	*stream_ << "</page>";

	*stream_ << "</ipe>\n";

	if (stream_->bad())
		throw std::exception("Failed to write SVG.");
}

void Ipe::writeDisk(const Disk& disk, Appearance appearance)
{
	if (disk.failure)
		appearance = Appearance::FAIL;

	writeCircle(disk.x, disk.y, disk.id, appearance);

	if (const auto* parent = graph_->findDisk(disk.parent)) {
		writeLine(disk.x, disk.y, parent->x, parent->y);
	}
}

void Ipe::writeCircle(float x, float y, int id, Appearance appearance)
{
	//float cx, cy;
	//grid_to_coord(x, y, 0, cx, cy);
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
	//float sx, sy, tx, ty;

	//grid_to_coord(x1, y1, 0, sx, sy);
	//grid_to_coord(x2, y2, 0, tx, ty);

	const Vec2 source = translate_.translate({ x1, y1 });
	const Vec2 target = translate_.translate({ x2, y2 });

	*stream_ << "<path stroke=\"black\" pen=\"0.4\">\n" << source.x << " " << source.y
		<< " m\n" << target.x << " " << target.y << " l\n</path>\n";
}
