// Various unit tests

#include "gtest/gtest.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include "embed.h"

std::ostream& operator<<(std::ostream& stream, GraphClass gc)
{
	switch (gc) {
	case GraphClass::CATERPILLAR: return stream << "CATERPILLAR";
	case GraphClass::LOBSTER: return stream << "LOBSTER";
	case GraphClass::OTHER: return stream << "OTHER";
	default: return stream << "[?]";
	}
}

TEST(Embed, classify_string)
{
	EdgeList edges{ {3, 5}, {4, 3}, {7, 4} }; // path-only caterpillar, no leaves

	const auto result = classify(edges);
	const auto& graph = result.first;

	EXPECT_EQ(GraphClass::CATERPILLAR, result.second) << "expected CATERPILLAR, but actually " << result.second;
	EXPECT_EQ(4, graph.spines().size()) << "expected 4 spine disks, but actually" << graph.spines().size();
	EXPECT_EQ(0, graph.branches().size()) << "expected no branch disks, but actually" << graph.branches().size();
	EXPECT_EQ(0, graph.leaves().size()) << "expected no leaf disks, but actually" << graph.leaves().size();
}

TEST(Embed, classify_caterpillar)
{
	EdgeList edges{ {3, 5}, {4, 3}, {7, 3} }; // caterpillar, 1 spine, 3 leaves

	const auto result = classify(edges);
	const auto& graph = result.first;

	EXPECT_EQ(GraphClass::CATERPILLAR, result.second) << "expected CATERPILLAR, but actually " << result.second;
	ASSERT_EQ(1, graph.spines().size()) << "expected 1 spine disk, but actually" << graph.spines().size();
	EXPECT_EQ(3, graph.spines()[0].id) << "expected spine disk is 3, but actually" << graph.spines()[0].id;
	EXPECT_EQ(3, graph.branches().size()) << "expected 3 branch disks, but actually" << graph.branches().size();
	EXPECT_EQ(0, graph.leaves().size()) << "expected no leaf disks, but actually" << graph.leaves().size();

	for (const auto& spine : graph.spines()) EXPECT_EQ(spine.depth, 0);
	for (const auto& branch : graph.branches()) EXPECT_EQ(branch.depth, 1);
	for (const auto& leaf : graph.leaves()) EXPECT_EQ(leaf.depth, 2);
}

TEST(Embed, classify_lobster)
{
	//   1 -- 5 -.
	//             3 -- 7 -- 8
	//   2 -- 4 -`
	EdgeList edges{ {3, 5}, {1, 5}, {7, 8}, {4, 3}, {4, 2}, {7, 3} }; // lobster, 1 spine, 3 branches, 3 leaves

	const auto result = classify(edges);
	const auto& graph = result.first;

	EXPECT_EQ(GraphClass::LOBSTER, result.second) << "expected LOBSTER, but actually " << result.second;
	EXPECT_EQ(1, graph.spines().size()) << "expected 1 spine disk, but actually" << graph.spines().size();
	EXPECT_EQ(3, graph.spines()[0].id) << "expected spine disk is 3, but actually" << graph.spines()[0].id;
	EXPECT_EQ(3, graph.branches().size()) << "expected 3 branch disks, but actually" << graph.branches().size();
	EXPECT_EQ(3, graph.leaves().size()) << "expected 3 leaf disks, but actually" << graph.leaves().size();

	for (const auto& spine : graph.spines()) EXPECT_EQ(spine.depth, 0);
	for (const auto& branch : graph.branches()) EXPECT_EQ(branch.depth, 1);
	for (const auto& leaf : graph.leaves()) EXPECT_EQ(leaf.depth, 2);
}

TEST(Embed, classify_stumped_lobster)
{
	//   1 -- 5 -.
	//            \       .- 8
	//        6 -- 3 -- 7
	//            /       `- 9
	//   2 -- 4 -`
	EdgeList edges{ {3, 6}, {3, 5}, {1, 5}, {7, 8}, {7, 9}, {4, 3}, {4, 2}, {7, 3} }; // lobster, 1 spine, 3 branches, 3 leaves

	const auto result = classify(edges);
	const auto& graph = result.first;

	EXPECT_EQ(GraphClass::LOBSTER, result.second) << "expected LOBSTER with stump, but actually " << result.second;
	EXPECT_EQ(1, graph.spines().size()) << "expected 1 spine disk, but actually" << graph.spines().size();
	EXPECT_EQ(3, graph.spines()[0].id) << "expected spine disk is 3, but actually" << graph.spines()[0].id;
	EXPECT_EQ(4, graph.branches().size()) << "expected 4 branch disks, but actually" << graph.branches().size();
	EXPECT_EQ(4, graph.leaves().size()) << "expected 4 leaf disks, but actually" << graph.leaves().size();

	for (const auto& spine : graph.spines()) EXPECT_EQ(spine.depth, 0);
	for (const auto& branch : graph.branches()) EXPECT_EQ(branch.depth, 1);
	for (const auto& leaf : graph.leaves()) EXPECT_EQ(leaf.depth, 2);
}
