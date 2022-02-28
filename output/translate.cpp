#include "translate.h"
#include <algorithm>
#include <cassert>

Translate::Translate(float scale) noexcept
	: left_(0), right_(0), top_(0), bottom_(0), margin_(0), scale_(scale)
{
}

Vec2 Translate::translate(Vec2 v) const noexcept
{
	const Vec2 offset{ margin_ - left_ * scale_, margin_ - top_ * scale_};
	return v * scale_ + offset;
}

void Translate::setLimits(float top, float right, float bottom, float left, float margin) noexcept
{
	assert(top < bottom);
	assert(left < right);

	top_ = top;
	right_ = right;
	bottom_ = bottom;
	left_ = left;
	margin_ = margin;
}

namespace
{

	// Find the least disk object according to the given comparator, or an all-0 disk.
	template<typename Iterator, typename Compare>
	Disk min_disk(Iterator begin, Iterator end, Compare compare)
	{
		auto min_elem = std::min_element(begin, end, compare);
		return min_elem == end ? Disk{ 0, 0, 0, 0 } : *min_elem;
	}

}

void Translate::setLimits(const DiskGraph& graph, float margin) noexcept
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
	const Disk& maxxDisk = std::min({ maxxSpine, maxxBranch, maxxLeaf }, greaterX);

	const auto greaterY = [](Disk a, Disk b) { return a.y > b.y; };

	const Disk& maxySpine = min_disk(graph.spines().begin(), graph.spines().end(), greaterY);
	const Disk& maxyBranch = min_disk(graph.branches().begin(), graph.branches().end(), greaterY);
	const Disk& maxyLeaf = min_disk(graph.leaves().begin(), graph.leaves().end(), greaterY);
	const Disk& maxyDisk = std::min({ maxySpine, maxyBranch, maxyLeaf }, greaterY);

	left_ = minxDisk.x - .5f;
	right_ = maxxDisk.x + .5f;
	top_ = minyDisk.y - .5f;
	bottom_ = maxyDisk.y + .5f;
	margin_ = margin;
}

float Translate::width() const noexcept
{
	return scale_ * (right_ - left_) + 2 * margin_;
}

float Translate::height() const noexcept
{
	return scale_ * (bottom_ - top_) + 2 * margin_;
}
