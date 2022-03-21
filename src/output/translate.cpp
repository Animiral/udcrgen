#include "translate.h"
#include <algorithm>
#include <numeric>
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
	// type of std::min and std::max
	using Select = const float& (*)(const float&, const float&);

	float merge(float value, const Disk& disk, Select select, float Disk::*member) noexcept
	{
		return (*select)(value, disk.*member);
	}
}

void Translate::setLimits(const DiskGraph& graph, float margin) noexcept
{
	const auto& disks = graph.disks();

	float minX = std::reduce(disks.begin(), disks.end(), 0.f,
		[](float value, const Disk& disk) { return merge(value, disk, &std::min<float>, &Disk::x); });
	float maxX = std::reduce(disks.begin(), disks.end(), 0.f,
		[](float value, const Disk& disk) { return merge(value, disk, &std::max<float>, &Disk::x); });
	float minY = std::reduce(disks.begin(), disks.end(), 0.f,
		[](float value, const Disk& disk) { return merge(value, disk, &std::min<float>, &Disk::y); });
	float maxY = std::reduce(disks.begin(), disks.end(), 0.f,
		[](float value, const Disk& disk) { return merge(value, disk, &std::max<float>, &Disk::y); });

	left_ = minX - .5f;
	right_ = maxX + .5f;
	top_ = minY - .5f;
	bottom_ = maxY + .5f;
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
