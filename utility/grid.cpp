#include "grid.h"
#include "geometry.h"

Grid::Grid(int length, int size)
	: length_(length), size_(size), values_((2*size+length) * (2*size+1), NODISK)
{
}

DiskId Grid::at(Coord coord) const
{
	std::size_t index = coordIndex(coord);

	if (index >= values_.size())
		return NODISK; // out of range
	
	else
		return values_.at(index);
}

Coord Grid::find(DiskId id) const
{
	for (int x = -size_; x < length_ + size_; x++) {
		for (int sly = -size_; sly < size_; sly++) {
			if (id == values_[coordIndex({ x,sly })])
				return Coord{ x, sly };
		}
	}

	throw std::exception("Disk not found in grid");
}

Coord Grid::step(Coord from, Rel rel) const
{
	switch (rel) {
	default:
	case Rel::HERE:      return from;
	case Rel::BACK:      return { from.x - 1, from.sly };
	case Rel::BACK_UP:   return { from.x - 1, from.sly + 1 };
	case Rel::BACK_DOWN: return { from.x, from.sly - 1 };
	case Rel::FWD_UP:    return { from.x, from.sly + 1 };
	case Rel::FWD_DOWN:  return { from.x + 1, from.sly - 1 };
	case Rel::FORWARD:   return { from.x + 1, from.sly };
	}
}

Vec2 Grid::vec(Coord coord) const
{
	return Vec2{ coord.x + coord.sly * .5f, coord.sly * 0.86602540378443864676372317075294f };
}

void Grid::put(Coord coord, DiskId id)
{
	values_.at(coordIndex(coord)) = id;
}

std::size_t Grid::coordIndex(Coord coord) const
{
	const int height = 2 * size_ + 1;
	return (coord.x + size_) * height + (coord.sly + size_);
}
