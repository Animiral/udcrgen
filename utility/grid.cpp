#include "grid.h"
#include "geometry.h"
#include <cassert>

Grid::Grid(std::size_t size)
	: size_(size), map_(size, coordHash)
{

}

Disk* Grid::at(Coord coord) const
{
	auto it = map_.find(coord);

	if (map_.end() == it) {
		return nullptr;
	} else {
		return it->second;
	}
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

void Grid::put(Coord coord, Disk& disk)
{
	auto it_inserted = map_.insert({ coord, &disk });

	if (!it_inserted.second) {
		throw std::exception("disks cannot overlap in grid");
	}
}

std::size_t Grid::coordHash(Coord coord) noexcept
{
	return (coord.sly << 16) + coord.x;
}
