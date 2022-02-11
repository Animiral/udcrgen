// Dynamic program unit tests

#include "gtest/gtest.h"
#include "dynamic.h"
#include "utility/grid.h"

TEST(Dynamic, fundament)
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

	Fundament fundament(grid, head);

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

/**
 * Test pushing and popping problems into and out of the dynamic queue.
 */
TEST(Dynamic, queue)
{
	std::vector<Disk> disks = {
		// id, parent, depth, children, embedded, grid_x, grid_sly
		{0, -1, 0, 6, true, 0, 0},
		{1,  0, 1, 0, true, -1, 0},
		{2,  0, 1, 0, true, 0, -1},
		{3,  0, 0, 0, true, 1, 0},
		{4,  3, 1, 0, false, 2, 0}
	};

	// p1 has depth 3, p2 has depth 4. Theferore p2 should be popped first.
	// p3 is equivalent to p1 and should not enter the queue.
	// (mirrored fundament, irrelevant branch head)
	DynamicProblem p1(disks), p2(disks), p3(disks);

	// prepare p1
	Grid solution1(5);
	for(int i = 0; i < 3; i++)
		solution1.put({ disks[i].grid_x, disks[i].grid_sly }, disks[i]);
	p1.setSolution(solution1, { 0, 0 }, { 0, -1 });

	// prepare p2
	Grid solution2(5);
	for (int i = 0; i < 4; i++)
		solution2.put({ disks[i].grid_x, disks[i].grid_sly }, disks[i]);
	p2.setSolution(solution2, { 1, 0 }, { 1, 0 });

	// prepare p3
	Grid solution3(5);
	solution3.put({ 0, 0 }, disks[0]);
	solution3.put({ -1, 1 }, disks[1]);
	solution3.put({ -1, 0 }, disks[2]);
	p3.setSolution(solution3, { 0, 0 }, { -1, 1 });

	EXPECT_NE(ProblemQueue::hash(p1.signature()), ProblemQueue::hash(p2.signature()));
	EXPECT_EQ(ProblemQueue::hash(p1.signature()), ProblemQueue::hash(p3.signature()));
	EXPECT_FALSE(ProblemQueue::equivalent(p1, p2));
	EXPECT_TRUE(ProblemQueue::equivalent(p1, p3));

	ProblemQueue queue;
	EXPECT_TRUE(queue.empty());

	queue.push(p1);
	queue.push(p2);
	queue.push(p3);

	ASSERT_FALSE(queue.empty());
	const DynamicProblem& out1 = queue.top();
	EXPECT_TRUE(ProblemQueue::equivalent(p2, out1));
	queue.pop();

	ASSERT_FALSE(queue.empty());
	const DynamicProblem& out2 = queue.top();
	EXPECT_TRUE(ProblemQueue::equivalent(p1, out2));
	queue.pop();

	EXPECT_TRUE(queue.empty());
}
