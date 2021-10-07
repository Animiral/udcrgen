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
	Grid grid(length, size);

	// populate grid
	for (int x = -size; x < length + size; x++) {
		for (int sly = -size; sly < size; sly++) {
			grid.at(x, sly) = x + 2 * sly;
		}
	}

	// read out grid
	for (int x = -size; x < length + size; x++) {
		for (int sly = -size; sly < size; sly++) {
			EXPECT_EQ(grid.at(x, sly), x + 2 * sly);
		}
	}
}
