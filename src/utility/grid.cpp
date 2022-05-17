#include "grid.h"
#include "geometry.h"
#include <cassert>

Grid::Grid(std::size_t size)
	: map_(size, coordHash)
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

void Grid::put(Coord coord, Disk& disk)
{
	auto it_inserted = map_.insert({ coord, &disk });
	assert(it_inserted.second);
}

std::size_t Grid::coordHash(Coord coord) noexcept
{
	return (coord.sly << 16) + coord.x;
}

void Grid::apply() const noexcept
{
	for (auto const& [coord, disk] : map_)
	{
		disk->grid_x = coord.x;
		disk->grid_sly = coord.sly;
		Vec2 diskVec = vec(coord);
		disk->x = diskVec.x;
		disk->y = diskVec.y;
		disk->embedded = true;
	}
}

std::size_t Grid::size() const noexcept
{
	return map_.size();
}
