#include "svg.h"
#include "utility/exception.h"
#include <algorithm>
#include <tuple>
#include <cassert>
#include <cstring>

Svg::Svg() noexcept :
	fileStream_(), stream_(),
	basePath_(), batchSize_(0), batchNr_(0), batchCount_(0),
	scale_(100.f), translate_(scale_)
{
}

Svg::Svg(std::ostream& stream) noexcept :
	fileStream_(), stream_(&stream),
	basePath_(), batchSize_(0), batchNr_(0), batchCount_(0),
	scale_(100.f), translate_(scale_)
{
}

Svg::Svg(const std::filesystem::path& path) :
	fileStream_(path), stream_(&fileStream_),
	basePath_(path), batchSize_(0), batchNr_(0), batchCount_(0),
	scale_(100.f), translate_(scale_)
{
	if (!fileStream_.is_open())
		throw OutputException(std::strerror(errno), path.string());
}

void Svg::open(std::filesystem::path basePath)
{
	assert(!fileStream_.is_open());

	fileStream_.open(basePath);

	if (!fileStream_.is_open())
		throw OutputException(std::strerror(errno), basePath.string());

	stream_ = &fileStream_;
	basePath_ = basePath;
	batchNr_ = 0;
	batchCount_ = 0;
}

void Svg::close()
{
	fileStream_.clear();
	fileStream_.close();

	if (fileStream_.fail())
		throw OutputException(std::strerror(errno));

	basePath_ = std::filesystem::path{};
	batchNr_ = 0;
	batchCount_ = 0;
}

void Svg::use(std::ostream& stream)
{
	stream_ = &stream;
	close();
	fileStream_ = std::ofstream{};
}

void Svg::setBatchSize(int batchSize) noexcept
{
	assert(batchSize >= 0);
	batchSize_ = batchSize;
}

void Svg::setScale(float scale) noexcept
{
	assert(scale >= 0);
	scale_ = scale;
	translate_ = Translate(scale);
}

void Svg::ensureBatch()
{
	if (0 >= batchSize_ || !fileStream_.is_open())
		return;

	if (batchCount_ >= batchSize_) {
		outro();

		fileStream_.clear();
		fileStream_.close();

		if (fileStream_.fail())
			throw OutputException(std::strerror(errno));

		batchNr_++;
		batchCount_ = 0;

		auto nextPath = basePath_;
		auto nextFilename = basePath_.stem().concat("_").concat(std::to_string(batchNr_))
			+= basePath_.extension();
		nextPath.replace_filename(nextFilename);

		fileStream_.open(nextPath);

		if (!fileStream_.is_open())
			throw OutputException(std::strerror(errno), nextPath.string());

		intro();
	}
}

void Svg::intro() const
{
	*stream_ <<
		"<html>\n"
		"\t<body>\n";

	if (stream_->fail())
		throw OutputException(std::strerror(errno));
}

void Svg::outro() const
{
	*stream_ << "\t</body>\n</html>\n";

	if (stream_->fail())
		throw OutputException(std::strerror(errno));
}

void Svg::write(const DiskGraph& graph, const std::string& label) const
{
	translate_.setLimits(graph, 10.f);

	openSvg(label);

	for (const Disk& disk : graph.disks())
		writeDisk(disk, graph);

	closeSvg();

	if (stream_->fail())
		throw OutputException(std::strerror(errno));

	batchCount_++;
}

void Svg::write(const Signature& signature, const std::string& label) const
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

	if (stream_->fail())
		throw OutputException(std::strerror(errno));

	batchCount_++;
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
}

void Svg::writeDisk(const Disk& disk, const DiskGraph& graph) const
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
