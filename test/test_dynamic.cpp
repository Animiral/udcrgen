// Dynamic program unit tests

#include "gtest/gtest.h"
#include "dynamic.h"
#include "utility/grid.h"

TEST(Dynamic, fundament_blocked)
{
	Disk dummy;
	Grid grid(6);
	grid.put({ 0, 0 }, dummy);
	grid.put({ -1, 0 }, dummy);
	grid.put({ -1, 1 }, dummy);
	grid.put({ 0, -1 }, dummy);
	grid.put({ 0, -2 }, dummy);
	grid.put({ 1, 0 }, dummy);
	Coord head{ 1, 0 };

	Fundament fundament(grid, head);
	EXPECT_EQ(fundament.mask, 0b00000'00000'00100'00011'00011);

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

	// modify
	fundament.block({ 0, -2 });
	EXPECT_TRUE(fundament.blocked({ 0, -2 }));
}

TEST(Dynamic, fundament_shift)
{
	// base | shifted_ru | shifted_r | shifted_rd
	//     x          -          -          -    
	//    x -        x -        - -        x -   
	//   x x -      x - -      x - -      - - -  
	//  x - - -    x x - -    - - - -    x x - -  
	// x x * - -  x - * - -  x x * - -  x x * - -  
	//  x x x -    x x - -    x x - -    x x - -  
	//   x x -      x x -      x - -      x x -  
	//    x x        x -        x -        x -   
	//     x          x          -          -    
	Fundament base;
	base.mask = 0b00001'00011'01101'11111'11111;

	Fundament shifted_ru = base;
	shifted_ru.shift(Dir::RIGHT_UP);
	EXPECT_EQ(shifted_ru.mask, 0b00000'00001'00011'01101'11111);

	Fundament shifted_r = base;
	shifted_r.shift(Dir::RIGHT);
	EXPECT_EQ(shifted_r.mask, 0b00000'00000'00001'00110'01111);

	Fundament shifted_rd = base;
	shifted_rd.shift(Dir::RIGHT_DOWN);
	EXPECT_EQ(shifted_rd.mask, 0b00000'00001'00110'01111'01111);
}

TEST(Dynamic, fundament_reachable)
{
	//     -
	//    - -
	//   x - -
	//  x - * -
	// x x x x -
	//  - x - -
	//   - x -
	//    - -
	//     -
	Grid grid(8);

	Disk dummy;
	grid.put({ 0, 0 }, dummy);
	grid.put({ -1, 0 }, dummy);
	grid.put({ -2, 0 }, dummy);
	grid.put({ -2, 1 }, dummy);
	grid.put({ -2, 2 }, dummy);
	grid.put({ 0, -1 }, dummy);
	grid.put({ 1, -2 }, dummy);
	grid.put({ 1, 0 }, dummy);

	Fundament fundament(grid, { 0, 0 });

	Coord from{ 0, 1 };
	Fundament reachable = fundament.reachable(from, 2);

	//     x
	//    - -
	//   x - -
	//  x - * -
	// x x x x -
	//  x x x x
	//   x x x
	//    x x
	//     x
	unsigned long expectedReachable = 0x1e77ff;
	EXPECT_EQ(expectedReachable, reachable.mask.to_ulong());

	reachable = fundament.reachableBySpine(from);

	//     x
	//    x x
	//   x x -
	//  x x * -
	// x x x x x
	//  x x x x
	//   x x x
	//    x x
	//     x
	expectedReachable = 0x13fffff;
	EXPECT_EQ(expectedReachable, reachable.mask.to_ulong());
}

/**
 * Test that subsets of fundaments are recognized as dominant.
 */
TEST(Dynamic, signature_dominates)
{
	//     x
	//    x -
	//   x x -
	//  x - - -
	// x x * - -
	//  x x x -
	//   x x -
	//    x x
	//     x
	Fundament fun1;
	fun1.mask = 0b00001'00011'01101'11111'11111;
	Signature sig1{ 10, fun1, {0, 0} };

	//     x
	//    x -
	//   x x -
	//  x x - -
	// x x * - -
	//  x x x -
	//   x x x
	//    x x
	//     x
	Fundament fun2;
	fun2.mask = 0b00001'00011'11111'11111'11111;
	Signature sig2{ 10, fun2, {0, 0} };

	//     x
	//    x -
	//   x x -
	//  x x - -
	// x x * - -
	//  x x - -
	//   x x -
	//    x x
	//     x
	Fundament fun3;
	fun3.mask = 0b00001'00011'00111'11111'11111;
	Signature sig3{ 10, fun3, {0, 0} };

	EXPECT_TRUE(sig1.dominates(sig1));
	EXPECT_TRUE(sig3.dominates(sig2));
	EXPECT_TRUE(sig1.dominates(sig2));
	EXPECT_FALSE(sig1.dominates(sig3));
	EXPECT_FALSE(sig3.dominates(sig1));

	Signature sig4{ 11, fun2, {0, 1} }; // unrelated depth, head
	EXPECT_FALSE(sig1.dominates(sig4));
	EXPECT_FALSE(sig4.dominates(sig1));
}

/**
 * Test that in the subproblems, the latest solution disks are
 * placed in the correct spot.
 * The fundament and heads must be updated.
 */
TEST(Dynamic, subproblem)
{
	// input lobster
	std::vector<Disk> disks(7); // TODO: use fully defined disks
	disks[0].depth = disks[5].depth = 0;

	for (int i : {1, 2, 3, 4, 6})
		disks[i].depth = 1;

	// partial solution - cut off not-yet solved disks
	Grid solution(7);
	solution.put({ 0, 0 }, disks[0]);
	solution.put({ -1, 0 }, disks[1]);
	solution.put({ -1, 1 }, disks[2]);
	solution.put({ 0, -1 }, disks[3]);
	solution.put({ 1, -1 }, disks[4]);
	
	Fundament fundament(solution, { 0, 0 });
	DiskGraph graph(move(disks));
	DynamicProblem problem(graph);
	GraphTraversal position(&graph.disks()[5], Configuration::EmbedOrder::DEPTH_FIRST);
	problem.setState(fundament, position, { 0, 0 }, { 1, -1 }, 5);
	EXPECT_EQ(problem.depth(), 5);

	std::vector<DynamicProblem> result = problem.subproblems();
	ASSERT_EQ(result.size(), 2);

	Coord expectedCoord{ 1, 0 };
	EXPECT_EQ(result[0].solution().at(expectedCoord), &graph.disks()[5]);
	EXPECT_EQ(result[0].spineHead(), expectedCoord);
	EXPECT_EQ(result[0].depth(), 6);

	expectedCoord = { 0, 1 };
	EXPECT_EQ(result[1].solution().at(expectedCoord), &graph.disks()[5]);
	EXPECT_EQ(result[1].spineHead(), expectedCoord);
	EXPECT_EQ(result[1].depth(), 6);
}

TEST(Dynamic, reachableEventually)
{
	//     -
	//    - -
	//   - 4 -
	//  - - * -
	// - 1 0 - -
	//  - 2 - -
	//   - - -
	//    - -
	//     -
	std::vector<Disk> disks(11);
	// Define disks as far as necessary for traversal
	for (int i : {0, 6, 7})
		disks[i].depth = 0;
	for (int i : {1, 2, 3, 8, 10})
		disks[i].depth = 1;
	for (int i : {4, 5, 9})
		disks[i].depth = 2;
	disks[5].parent = &disks[3];
	disks[3].parent = &disks[0];
	disks[0].nextSibling = &disks[6];
	disks[6].nextSibling = &disks[7];
	disks[7].child = &disks[8];
	disks[8].child = &disks[9];
	disks[9].nextSibling = &disks[10];

	int depth = 5;

	// partial solution - cut off not-yet solved disks
	Grid solution(10);
	solution.put({ 0, 0 }, disks[0]);
	solution.put({ -1, 0 }, disks[1]);
	solution.put({ 0, -1 }, disks[2]);
	solution.put({ 0, 1 }, disks[3]);
	solution.put({ -1, 2 }, disks[4]);

	Fundament base(solution, { 0, 0 });
	Coord head{ 0, 1 };
	GraphTraversal position(&disks[5], Configuration::EmbedOrder::DEPTH_FIRST);
	Fundament actual = reachableEventually(base, head, position, {});

	//     x
	//    x -
	//   x x -
	//  x - x -
	// x x x - -
	//  x x - -
	//   - - -
	//    - -
	//     -
	unsigned long expected = 0x1394e3;
	EXPECT_EQ(actual.mask.to_ulong(), expected);
}

/**
 * Test pushing and popping problems into and out of the dynamic queue.
 */
TEST(Dynamic, queue)
{
	std::vector<Disk> disks(5);
	disks[0].depth = disks[3].depth = 0;
	disks[1].depth = disks[2].depth = disks[4].depth = 1;
	DiskGraph graph(move(disks));

	// p1 has depth 3, p2 has depth 4. Theferore p2 should be popped first.
	// p3 is equivalent to p1 and should not enter the queue.
	// (mirrored fundament, irrelevant branch head)
	DynamicProblem p1(graph), p2(graph), p3(graph);

	// prepare p1
	//     -
	//    - -
	//   - - -      (0)
	//  - - - -      |
	// - 1 0 - -     3
	//  - - 2 -      |
	//   - - -       4
	//    - -
	//     -
	Grid solution1(5);
	solution1.put({ 0, 0 }, graph.disks()[0]);
	solution1.put({ -1, 0 }, graph.disks()[1]);
	solution1.put({ 1, -1 }, graph.disks()[2]);
	Fundament fundament1(solution1, { 0, 0 });
	GraphTraversal position1(&graph.disks()[3], Configuration::EmbedOrder::DEPTH_FIRST);
	p1.setState(fundament1, position1, { 0, 0 }, { 0, -1 }, 3);

	// prepare p2
	//     -
	//    - -
	//   - - -      (3)
	//  - - - -      |
	// 1 0 3 - -     |
	//  - 2 - -      |
	//   - - -       4
	//    - -
	//     -
	Grid solution2(5);
	solution2.put({ 0, 0 }, graph.disks()[0]);
	solution2.put({ -1, 0 }, graph.disks()[1]);
	solution2.put({ 1, -1 }, graph.disks()[2]);
	solution2.put({ 1, 0 }, graph.disks()[3]);
	Fundament fundament2(solution2, { 1, 0 });
	GraphTraversal position2(&graph.disks()[4], Configuration::EmbedOrder::DEPTH_FIRST);
	p2.setState(fundament2, position2, { 1, 0 }, { 1, 0 }, 4);

	// prepare p3
	//     -
	//    - -
	//   - - -      (0)
	//  - 1 2 -      |
	// - - 0 - -     3
	//  - - - -      |
	//   - - -       4
	//    - -
	//     -
	Grid solution3(5);
	solution3.put({ 0, 0 }, graph.disks()[0]);
	solution3.put({ -1, 1 }, graph.disks()[1]);
	solution3.put({ 0, 1 }, graph.disks()[2]);
	Fundament fundament3(solution3, { 0, 0 });
	p3.setState(fundament3, position1, { 0, 0 }, { -1, 0 }, 3);

	// p1 and p3 are equal because of mirroring and reachability
	EXPECT_FALSE(ProblemQueue::equivalent(p1, p2));
	EXPECT_TRUE(ProblemQueue::equivalent(p1, p3));

	ProblemQueue queue(5);
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

/**
 * Test that the queue applies dominance of signatures when pushing
 * and popping problems.
 */
TEST(Dynamic, queue_dominant)
{
	std::vector<Disk> disks(6);
	disks[0].depth = disks[2].depth = disks[3].depth = 0;
	disks[1].depth = disks[4].depth = 1;
	disks[5].depth = 2;
	DiskGraph graph(move(disks));

	DynamicProblem p1(graph), p2(graph);

	// prepare p1
	//     -
	//    - -
	//   - - -      (3)
	//  1 - - -      |
	// 0 2 3 - -     4
	//  - - - -      |
	//   - - -       5
	//    - -
	//     -
	Grid solution1(6);
	solution1.put({ 0, 0 }, graph.disks()[0]);
	solution1.put({ 0, 1 }, graph.disks()[1]);
	solution1.put({ 1, 0 }, graph.disks()[2]);
	solution1.put({ 2, 0 }, graph.disks()[3]);
	Fundament fundament1(solution1, { 2, 0 });
	GraphTraversal position(&graph.disks()[4], Configuration::EmbedOrder::DEPTH_FIRST);
	p1.setState(fundament1, position, { 2, 0 }, { 2, 0 }, 4);

	// prepare p2 - "better" than p1
	//     -
	//    - -
	//   - - -      (3)
	//  - - - -      |
	// 0 2 3 - -     4
	//  - - - -      |
	//   - - -       5
	//    - -
	//     -
	Grid solution2(6);
	solution2.put({ 0, 0 }, graph.disks()[0]);
	solution2.put({ 0, -1 }, graph.disks()[1]);
	solution2.put({ 1, 0 }, graph.disks()[2]);
	solution2.put({ 2, 0 }, graph.disks()[3]);
	Fundament fundament2(solution2, { 2, 0 });
	p2.setState(fundament2, position, { 2, 0 }, { 2, 0 }, 4);

	ProblemQueue queue(6);
	queue.push(p2);
	queue.push(p1); // better p2 prevents insertion of p1

	const DynamicProblem& out2 = queue.top();
	EXPECT_TRUE(ProblemQueue::equivalent(p2, out2));

	queue.pop();
	ASSERT_TRUE(queue.empty());
}

TEST(Dynamic, embed_dynamic)
{
	const auto NB = Lobster::NO_BRANCH;
	Lobster lobster({ {2, 2, 2, 1, 1}, {2, 2, 2, NB, NB} });
	DiskGraph graph = DiskGraph::fromLobster(lobster);
	DynamicProblemEmbedder embedder;
	EXPECT_TRUE(embedder.embed(graph));
}
