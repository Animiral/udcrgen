#include "grid.h"

Grid::Grid(int length, int size)
	: length_(length), size_(size), values_((2*size+length) * (2*size+1))
{
}

int& Grid::at(int x, int sly)
{
	const int height = 2 * size_ + 1;
	return values_.at((x + size_)*height + (sly + size_));
}
