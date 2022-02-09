// Dynamic program unit tests

#include "gtest/gtest.h"
#include "dynamic.h"
#include "utility/grid.h"

TEST(Dynamic, makeFundament)
{
	std::vector<Disk> disks = {
		// id, parent, depth, children, embedded, grid_x, grid_sly
		{0, -1, 0, 4, true, 0, 0},
		{1,  0, 1, 0, true, -1, 0},
		{2,  0, 1, 0, true, -1, 1},
		{3,  0, 1, 0, true, 0, -1},
		{4,  3, 2, 0, true, 0, -2},
		{5, 0, 0, 0, true, 1, 0}
	};

	Grid grid(6);
	for (std::size_t i = 0; i < 6; i++) {
		grid.put({ disks[i].grid_x, disks[i].grid_sly }, disks[i]);
	}

	Coord head{ 1, 0 };

	Fundament fundament = makeFundament(grid, head);

	EXPECT_TRUE(fundament.blocked({ 0, 0 })); // head (should always be true)
	EXPECT_TRUE(fundament.blocked({ -1, 0 })); // disk 0 (spine)
	EXPECT_TRUE(fundament.blocked({ -2, 0 })); // disk 1 (branch)
	EXPECT_TRUE(fundament.blocked({ -2, 1 })); // disk 2 (branch)
	EXPECT_TRUE(fundament.blocked({ -1, -1 })); // disk 3 (branch)

	// all other grid locations in range
	EXPECT_FALSE(fundament.blocked({ -2, 2 }));
	EXPECT_FALSE(fundament.blocked({ -1, 2 }));
	EXPECT_FALSE(fundament.blocked({ 0, 2 }));

	EXPECT_FALSE(fundament.blocked({ -1, 1 }));
	EXPECT_FALSE(fundament.blocked({ 0, 1 }));
	EXPECT_FALSE(fundament.blocked({ 1, 1 }));

	EXPECT_FALSE(fundament.blocked({ 1, 0 }));
	EXPECT_FALSE(fundament.blocked({ 2, 0 }));

	EXPECT_FALSE(fundament.blocked({ 0, -1 }));
	EXPECT_FALSE(fundament.blocked({ 1, -1 }));
	EXPECT_FALSE(fundament.blocked({ 2, -1 }));

	EXPECT_FALSE(fundament.blocked({ 0, -2 }));
	EXPECT_FALSE(fundament.blocked({ 1, -2 }));
	EXPECT_FALSE(fundament.blocked({ 2, -2 }));
}

/**
 * Test that in the subproblems, the latest solution disks are
 * placed in the correct spot.
 * The fundament and heads must be updated.
 */
TEST(Dynamic, subproblem)
{
	// input lobster
	std::vector<Disk> disks = {
		// id, parent, depth, children, embedded, grid_x, grid_sly
		{0, -1, 0, 6, true, 0, 0},
		{1,  0, 1, 0, true, -1, 0},
		{2,  0, 1, 0, true, -1, 1},
		{3,  0, 1, 0, true, 0, -1},
		{4,  0, 1, 0, true, 1, -1},
		{5,  0, 0},
		{6,  5, 1}
	};

	// partial solution - cut off not-yet solved disks
	Grid solution(7);
	for (std::size_t i = 0; i < 5; i++) {
		solution.put({ disks[i].grid_x, disks[i].grid_sly }, disks[i]);
	}

	DynamicProblem problem(disks);
	problem.setSolution(solution, { 0, 0 }, { 1, -1 });
	EXPECT_EQ(problem.depth(), 5);

	std::vector<DynamicProblem> result = problem.subproblems();
	ASSERT_EQ(result.size(), 2);

	Coord expectedCoord{ 0, 1 };
	EXPECT_EQ(result[0].solution().at(expectedCoord), &disks[5]);
	EXPECT_EQ(result[0].spineHead(), expectedCoord);
	EXPECT_EQ(result[0].depth(), 6);

	expectedCoord = { 1, 0 };
	EXPECT_EQ(result[1].solution().at(expectedCoord), &disks[5]);
	EXPECT_EQ(result[1].spineHead(), expectedCoord);
	EXPECT_EQ(result[1].depth(), 6);
}
