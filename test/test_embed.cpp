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

// a simple easy example caterpillar: spine [0] with 3 leaves, spine [1] with 4 leaves
DiskGraph make_caterpillar()
{
	DiskGraph graph{ 2, 7, 0 };
	auto& spines = graph.spines();
	auto& branches = graph.branches();
	Disk* disk;

	for (DiskId id = 0; id < 9; id++) {
		if (id < 2) {
			disk = &spines[id];
			disk->parent = id - 1;
			disk->depth = 0;
		}
		else {
			disk = &branches[id - 2];
			disk->parent = id < 5 ? 0 : 1;
			disk->depth = 1;
		}

		disk->id = id;
	}

	return graph;
}

TEST(Embed, embed_proper)
{
	auto graph = make_caterpillar();
	auto embedder = ProperEmbedder{};

	// execute
	embed(graph, embedder, Configuration::EmbedOrder::LBS);

	auto& spines = graph.spines();
	auto& branches = graph.branches();
	EXPECT_NEAR(spines[0].x, 0, 0.01);        EXPECT_NEAR(spines[0].y, 0, 0.01);
	EXPECT_NEAR(spines[1].x, 1, 0.01);        EXPECT_NEAR(spines[1].y, 0, 0.01);
	EXPECT_NEAR(branches[0].x, -1, 0.01);     EXPECT_NEAR(branches[0].y, 0, 0.01);
	EXPECT_NEAR(branches[1].x, -0.395, 0.01); EXPECT_NEAR(branches[1].y, -0.91889f, 0.01);
	EXPECT_NEAR(branches[2].x, -0.395, 0.01); EXPECT_NEAR(branches[2].y, 0.91889f, 0.01);
	EXPECT_NEAR(branches[3].x, .705, 0.01);   EXPECT_NEAR(branches[3].y, -0.95466f, 0.01);
	EXPECT_NEAR(branches[4].x, .705, 0.01);   EXPECT_NEAR(branches[4].y, 0.95466f, 0.01);
	EXPECT_NEAR(branches[5].x, 1.7605, 0.01); EXPECT_NEAR(branches[5].y, -0.6481f, 0.01);
	EXPECT_NEAR(branches[6].x, 1.7605, 0.01); EXPECT_NEAR(branches[6].y, 0.6481f, 0.01);
}

// a simple easy example lobster: spine [0] with 3 branches (2, 1, 1 leaves), spine [1] with 4 branches (first has 1 leaf)
DiskGraph make_lobster()
{
	DiskGraph graph{ 2, 7, 5 };
	auto& spines = graph.spines();
	auto& branches = graph.branches();
	auto& leaves = graph.leaves();
	Disk* disk;

	for (DiskId id = 0; id < 14; id++) {
		if (id < 2) {
			disk = &spines[id];
			disk->parent = id - 1;
			disk->depth = 0;
		}
		else if (id < 9) {
			disk = &branches[id - 2];
			disk->parent = id < 5 ? 0 : 1;
			disk->depth = 1;
		}
		else {
			disk = &leaves[id - 9];
			disk->parent = id < 10 ? 2 : id-8;
			disk->depth = 2;
		}

		disk->id = id;
	}

	return graph;
}

TEST(Embed, embed_weak)
{
	auto graph = make_lobster();
	auto embedder = WeakEmbedder{ graph };

	// execute, leaves first
	embed(graph, embedder, Configuration::EmbedOrder::LBS);

	auto& spines = graph.spines();
	auto& branches = graph.branches();
	auto& leaves = graph.leaves();
	EXPECT_NEAR(spines[0].x, 0, 0.01);      EXPECT_NEAR(spines[0].y, 0, 0.01);         EXPECT_EQ(spines[0].grid_x, 0);    EXPECT_EQ(spines[0].grid_sly, 0);
	EXPECT_NEAR(spines[1].x, 1, 0.01);      EXPECT_NEAR(spines[1].y, 0, 0.01);         EXPECT_EQ(spines[1].grid_x, 1);    EXPECT_EQ(spines[1].grid_sly, 0);
	EXPECT_NEAR(branches[0].x, -1, 0.01);   EXPECT_NEAR(branches[0].y, 0, 0.01);       EXPECT_EQ(branches[0].grid_x, -1); EXPECT_EQ(branches[0].grid_sly, 0);
	EXPECT_NEAR(branches[1].x, -0.5, 0.01); EXPECT_NEAR(branches[1].y, -0.866f, 0.01); EXPECT_EQ(branches[1].grid_x, 0);  EXPECT_EQ(branches[1].grid_sly, -1);
	EXPECT_NEAR(branches[2].x, -0.5, 0.01); EXPECT_NEAR(branches[2].y, 0.866f, 0.01);  EXPECT_EQ(branches[2].grid_x, -1); EXPECT_EQ(branches[2].grid_sly, 1);
	EXPECT_NEAR(branches[3].x, .5, 0.01);   EXPECT_NEAR(branches[3].y, 0.866f, 0.01);  EXPECT_EQ(branches[3].grid_x, 0);  EXPECT_EQ(branches[3].grid_sly, 1);
	EXPECT_NEAR(branches[4].x, .5, 0.01);   EXPECT_NEAR(branches[4].y, -0.866f, 0.01); EXPECT_EQ(branches[4].grid_x, 1);  EXPECT_EQ(branches[4].grid_sly, -1);
	EXPECT_NEAR(branches[5].x, 1.5, 0.01);  EXPECT_NEAR(branches[5].y, -0.866f, 0.01); EXPECT_EQ(branches[5].grid_x, 2);  EXPECT_EQ(branches[5].grid_sly, -1);
	EXPECT_NEAR(branches[6].x, 1.5, 0.01);  EXPECT_NEAR(branches[6].y, 0.866f, 0.01);  EXPECT_EQ(branches[6].grid_x, 1);  EXPECT_EQ(branches[6].grid_sly, 1);
	EXPECT_NEAR(leaves[0].x, -2, 0.01);     EXPECT_NEAR(leaves[0].y, 0, 0.01);         EXPECT_EQ(leaves[0].grid_x, -2);   EXPECT_EQ(leaves[0].grid_sly, 0);
	EXPECT_NEAR(leaves[1].x, -1.5, 0.01);   EXPECT_NEAR(leaves[1].y, 0.866f, 0.01);    EXPECT_EQ(leaves[1].grid_x, -2);   EXPECT_EQ(leaves[1].grid_sly, 1);
	EXPECT_NEAR(leaves[2].x, -1.5, 0.01);   EXPECT_NEAR(leaves[2].y, -0.866f, 0.01);   EXPECT_EQ(leaves[2].grid_x, -1);   EXPECT_EQ(leaves[2].grid_sly, -1);
	EXPECT_NEAR(leaves[3].x, -1, 0.01);     EXPECT_NEAR(leaves[3].y, 1.732f, 0.01);    EXPECT_EQ(leaves[3].grid_x, -2);   EXPECT_EQ(leaves[3].grid_sly, 2);
	EXPECT_NEAR(leaves[4].x, 0, 0.01);      EXPECT_NEAR(leaves[4].y, 1.732f, 0.01);    EXPECT_EQ(leaves[4].grid_x, -1);   EXPECT_EQ(leaves[4].grid_sly, 2);
}

TEST(Embed, embed_weak_sbl)
{
	auto graph = make_lobster();
	auto embedder = WeakEmbedder{ graph };

	// execute, spine first
	embed(graph, embedder, Configuration::EmbedOrder::SBL);

	auto& spines = graph.spines();
	auto& branches = graph.branches();
	auto& leaves = graph.leaves();
	EXPECT_NEAR(spines[0].x, 0, 0.01);      EXPECT_NEAR(spines[0].y, 0, 0.01);         EXPECT_EQ(spines[0].grid_x, 0);    EXPECT_EQ(spines[0].grid_sly, 0);
	EXPECT_NEAR(spines[1].x, 1, 0.01);      EXPECT_NEAR(spines[1].y, 0, 0.01);         EXPECT_EQ(spines[1].grid_x, 1);    EXPECT_EQ(spines[1].grid_sly, 0);
	EXPECT_NEAR(branches[0].x, -1, 0.01);   EXPECT_NEAR(branches[0].y, 0, 0.01);       EXPECT_EQ(branches[0].grid_x, -1); EXPECT_EQ(branches[0].grid_sly, 0);
	EXPECT_NEAR(branches[1].x, -0.5, 0.01); EXPECT_NEAR(branches[1].y, 0.866f, 0.01);  EXPECT_EQ(branches[1].grid_x, -1); EXPECT_EQ(branches[1].grid_sly, 1);
	EXPECT_NEAR(branches[2].x, -0.5, 0.01); EXPECT_NEAR(branches[2].y, -0.866f, 0.01); EXPECT_EQ(branches[2].grid_x, 0);  EXPECT_EQ(branches[2].grid_sly, -1);
	EXPECT_NEAR(branches[3].x, .5, 0.01);   EXPECT_NEAR(branches[3].y, 0.866f, 0.01);  EXPECT_EQ(branches[3].grid_x, 0);  EXPECT_EQ(branches[3].grid_sly, 1);
	EXPECT_NEAR(branches[4].x, .5, 0.01);   EXPECT_NEAR(branches[4].y, -0.866f, 0.01); EXPECT_EQ(branches[4].grid_x, 1);  EXPECT_EQ(branches[4].grid_sly, -1);
	EXPECT_NEAR(branches[5].x, 1.5, 0.01);  EXPECT_NEAR(branches[5].y, 0.866f, 0.01);  EXPECT_EQ(branches[5].grid_x, 1);  EXPECT_EQ(branches[5].grid_sly, 1);
	EXPECT_NEAR(branches[6].x, 1.5, 0.01);  EXPECT_NEAR(branches[6].y, -0.866f, 0.01); EXPECT_EQ(branches[6].grid_x, 2);  EXPECT_EQ(branches[6].grid_sly, -1);
	EXPECT_NEAR(leaves[0].x, -2, 0.01);     EXPECT_NEAR(leaves[0].y, 0, 0.01);         EXPECT_EQ(leaves[0].grid_x, -2);   EXPECT_EQ(leaves[0].grid_sly, 0);
	EXPECT_NEAR(leaves[1].x, -1.5, 0.01);   EXPECT_NEAR(leaves[1].y, 0.866f, 0.01);    EXPECT_EQ(leaves[1].grid_x, -2);   EXPECT_EQ(leaves[1].grid_sly, 1);
	EXPECT_NEAR(leaves[2].x, -1, 0.01);     EXPECT_NEAR(leaves[2].y, 1.732f, 0.01);    EXPECT_EQ(leaves[2].grid_x, -2);   EXPECT_EQ(leaves[2].grid_sly, 2);
	EXPECT_NEAR(leaves[3].x, -1.5, 0.01);   EXPECT_NEAR(leaves[3].y, -0.866f, 0.01);   EXPECT_EQ(leaves[3].grid_x, -1);   EXPECT_EQ(leaves[3].grid_sly, -1);
	EXPECT_NEAR(leaves[4].x, 0, 0.01);      EXPECT_NEAR(leaves[4].y, 1.732f, 0.01);    EXPECT_EQ(leaves[4].grid_x, -1);   EXPECT_EQ(leaves[4].grid_sly, 2);
}
