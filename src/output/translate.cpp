#include "translate.h"
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

void Translate::setLimits(const DiskGraph& graph, float margin) noexcept
{
	float minX = 0.f, minY = 0.f, maxX = 0.f, maxY = 0.f;

	for (const Disk& d : graph.disks()) {
		if (d.x < minX) minX = d.x;
		if (d.x > maxX) maxX = d.x;
		if (d.y < minY) minY = d.y;
		if (d.y > maxY) maxY = d.y;
	}

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
