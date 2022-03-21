// unit tests for graph classification and embedding

#include "gtest/gtest.h"
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
	EXPECT_EQ(4, graph.disks().size()) << "expected 4 disks, but actually" << graph.disks().size();
	for (const Disk& d : graph.disks()) {
		EXPECT_EQ(0, d.depth) << "expected spines only";
	}
}

TEST(Embed, classify_caterpillar)
{
	EdgeList edges{ {3, 5}, {4, 3}, {7, 3} }; // caterpillar, 1 spine, 3 leaves

	const auto result = classify(edges);
	const auto& graph = result.first;

	EXPECT_EQ(GraphClass::CATERPILLAR, result.second) << "expected CATERPILLAR, but actually " << result.second;
	ASSERT_EQ(1, graph.length()) << "expected 1 spine disk, but actually " << graph.length();
	EXPECT_EQ(4, graph.size()) << "expected 3 disks, but actually " << graph.size();
	EXPECT_EQ(0, graph.disks()[0].depth) << "expected depth 0 (spine) as first disk, but actually " << graph.disks()[0].depth;
	EXPECT_EQ(3, graph.disks()[0].id) << "expected spine disk is 3, but actually " << graph.disks()[0].id;

	for (int i = 1; i < 4; i++)
		EXPECT_EQ(1, graph.disks()[i].depth) << "expected depth 1 (branch) for disk, but actually " << graph.disks()[i].depth;
}

TEST(Embed, classify_lobster)
{
	//   1 -- 5 -.
	//             3 -- 7 -- 8
	//   2 -- 4 -`
	EdgeList edges{ {3, 5}, {1, 5}, {7, 8}, {4, 3}, {4, 2}, {7, 3} }; // lobster, 1 spine, 3 branches, 3 leaves

	auto result = classify(edges);
	auto& graph = result.first;
	graph.reorder(Configuration::EmbedOrder::BREADTH_FIRST);

	EXPECT_EQ(GraphClass::LOBSTER, result.second) << "expected LOBSTER, but actually " << result.second;
	EXPECT_EQ(1, graph.length()) << "expected 1 spine disk, but actually " << graph.length();
	EXPECT_EQ(7, graph.size()) << "expected 7 disks, but actually " << graph.size();
	EXPECT_EQ(0, graph.disks()[0].depth) << "expected depth 0 (spine) as first disk, but actually " << graph.disks()[0].depth;
	EXPECT_EQ(3, graph.disks()[0].id) << "expected spine disk is 3, but actually " << graph.disks()[0].id;

	for (int i = 1; i < 4; i++)
		EXPECT_EQ(1, graph.disks()[i].depth) << "expected depth 1 (branch) for disk, but actually " << graph.disks()[i].depth;

	for (int i = 4; i < 7; i++)
		EXPECT_EQ(2, graph.disks()[i].depth) << "expected depth 2 (leaf) for disk, but actually " << graph.disks()[i].depth;
}

TEST(Embed, classify_stumped_lobster)
{
	//   1 -- 5 -.
	//            \       .- 8
	//        6 -- 3 -- 7
	//            /       `- 9
	//   2 -- 4 -`
	EdgeList edges{ {3, 6}, {3, 5}, {1, 5}, {7, 8}, {7, 9}, {4, 3}, {4, 2}, {7, 3} }; // lobster, 1 spine, 3 branches, 3 leaves

	auto result = classify(edges);
	auto& graph = result.first;
	graph.reorder(Configuration::EmbedOrder::BREADTH_FIRST);

	EXPECT_EQ(GraphClass::LOBSTER, result.second) << "expected LOBSTER with stump, but actually " << result.second;
	EXPECT_EQ(1, graph.length()) << "expected 1 spine disk, but actually" << graph.length();
	EXPECT_EQ(9, graph.size()) << "expected 7 disks, but actually " << graph.size();
	EXPECT_EQ(0, graph.disks()[0].depth) << "expected depth 0 (spine) as first disk, but actually " << graph.disks()[0].depth;
	EXPECT_EQ(3, graph.disks()[0].id) << "expected spine disk is 3, but actually " << graph.disks()[0].id;

	for (int i = 1; i < 5; i++)
		EXPECT_EQ(1, graph.disks()[i].depth) << "expected depth 1 (branch) for disk, but actually " << graph.disks()[i].depth;

	for (int i = 5; i < 9; i++)
		EXPECT_EQ(2, graph.disks()[i].depth) << "expected depth 2 (leaf) for disk, but actually " << graph.disks()[i].depth;
}
