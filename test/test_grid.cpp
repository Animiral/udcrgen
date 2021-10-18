// Unit tests for the triangular grid

#include "gtest/gtest.h"
#include "utility/grid.h"

/**
 * Just store/read the whole grid.
 */
TEST(Grid, Grid_access)
{
	const int length = 5;
	const int size = 2;
	const std::size_t disk_count = 2 * size * (length + 2 * size);
	Disk disks[disk_count];
	Grid grid(disk_count);

	// populate grid
	for (int x = -size; x < length + size; x++) {
		for (int sly = -size; sly < size; sly++) {
			std::size_t i = (x + size) * 2 * size + (sly + size);
			grid.put({ x, sly }, disks[i]);
		}
	}

	// read out grid
	for (int x = -size; x < length + size; x++) {
		for (int sly = -size; sly < size; sly++) {
			std::size_t i = (x + size) * 2 * size + (sly + size);
			EXPECT_EQ(grid.at({ x, sly }), &disks[i]);
		}
	}
}
