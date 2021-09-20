#include "translate.h"
#include <algorithm>

namespace
{

// Find the least disk object according to the given comparator, or an all-0 disk.
template<typename Iterator, typename Compare>
Disk min_disk(Iterator begin, Iterator end, Compare compare);

// Find the limits of the graph in layout units
void calculateExtents(const DiskGraph& graph, float& left, float& right, float& top, float& bottom) noexcept;

}

Translate::Translate(const DiskGraph& graph, float margin, float scale) noexcept
	: margin_(margin), scale_(scale)
{
	calculateExtents(graph, left_, right_, top_, bottom_);
}

Vec2 Translate::translate(Vec2 v) const noexcept
{
	const Vec2 offset{ margin_ - left_ * scale_, margin_ - top_ * scale_};
	return v * scale_ + offset;
}

namespace
{

template<typename Iterator, typename Compare>
Disk min_disk(Iterator begin, Iterator end, Compare compare)
{
	auto min_elem = std::min_element(begin, end, compare);
	return min_elem == end ? Disk{ 0, 0, 0, 0 } : *min_elem;
}

void calculateExtents(const DiskGraph& graph, float& left, float& right, float& top, float& bottom) noexcept
{
	const auto lessX = [](Disk a, Disk b) { return a.x < b.x; };

	const Disk& minxSpine = min_disk(graph.spines().begin(), graph.spines().end(), lessX);
	const Disk& minxBranch = min_disk(graph.branches().begin(), graph.branches().end(), lessX);
	const Disk& minxLeaf = min_disk(graph.leaves().begin(), graph.leaves().end(), lessX);
	const Disk& minxDisk = std::min({ minxSpine, minxBranch, minxLeaf }, lessX);

	const auto lessY = [](Disk a, Disk b) { return a.y < b.y; };

	const Disk& minySpine = min_disk(graph.spines().begin(), graph.spines().end(), lessY);
	const Disk& minyBranch = min_disk(graph.branches().begin(), graph.branches().end(), lessY);
	const Disk& minyLeaf = min_disk(graph.leaves().begin(), graph.leaves().end(), lessY);
	const Disk& minyDisk = std::min({ minySpine, minyBranch, minyLeaf }, lessY);

	const auto greaterX = [](Disk a, Disk b) { return a.x > b.x; };

	const Disk& maxxSpine = min_disk(graph.spines().begin(), graph.spines().end(), greaterX);
	const Disk& maxxBranch = min_disk(graph.branches().begin(), graph.branches().end(), greaterX);
	const Disk& maxxLeaf = min_disk(graph.leaves().begin(), graph.leaves().end(), greaterX);
	const Disk& maxxDisk = std::min({ minxSpine, minxBranch, minxLeaf }, greaterX);

	const auto greaterY = [](Disk a, Disk b) { return a.y < b.y; };

	const Disk& maxySpine = min_disk(graph.spines().begin(), graph.spines().end(), greaterY);
	const Disk& maxyBranch = min_disk(graph.branches().begin(), graph.branches().end(), greaterY);
	const Disk& maxyLeaf = min_disk(graph.leaves().begin(), graph.leaves().end(), greaterY);
	const Disk& maxyDisk = std::min({ minySpine, minyBranch, minyLeaf }, greaterY);

	left = minxDisk.x - .5f;
	right = maxxDisk.x + .5f;
	top = minyDisk.y - .5f;
	bottom = maxyDisk.y + .5f;
}

}
