// Embed heuristic unit tests

#include "gtest/gtest.h"
#include "heuristic.h"

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
	embed(graph, embedder, Configuration::EmbedOrder::DEPTH_FIRST);

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
			disk->parent = id < 10 ? 2 : id - 8;
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
	embed(graph, embedder, Configuration::EmbedOrder::DEPTH_FIRST);

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

TEST(Embed, embed_weak_breadthfirst)
{
	auto graph = make_lobster();
	auto embedder = WeakEmbedder{ graph };

	// execute, spine first
	embed(graph, embedder, Configuration::EmbedOrder::BREADTH_FIRST);

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

// In this lobster, the branch on the second spine must be placed further out.
DiskGraph make_lobster_for_space_heuristic()
{
	DiskGraph graph{ 1, 3, 7 };
	auto& spines = graph.spines();
	auto& branches = graph.branches();
	auto& leaves = graph.leaves();
	Disk* disk;

	// spine
	disk = &spines[0];
	disk->id = 0;
	disk->parent = -1;
	disk->children = 3;
	disk->depth = 0;

	// branches
	for (DiskId id = 1; id < 4; id++) {
		disk = &branches[id - 1];
		disk->id = id;
		disk->parent = 0;
		disk->depth = 1;
	}
	branches[0].children = 3;
	branches[2].children = 4;

	// leaves
	for (DiskId id = 4; id < 11; id++) {
		disk = &leaves[id - 4];
		disk->id = id;
		disk->parent = id > 6 ? 3 : 1;
		disk->depth = 2;
	}

	return graph;
}

// The weak embedder must observe the "space heuristic", i.e. place the next
// branch with enough leftover room to allow for all child vertices.
TEST(Embed, space_heuristic)
{
	auto graph = make_lobster_for_space_heuristic();

	// fix embedding until we need to apply the space heuristic
	Disk* d;
	d = &graph.spines()[0];   d->embedded = true; d->grid_x = 0; d->grid_sly = 0;
	d = &graph.branches()[0]; d->embedded = true; d->grid_x = -1; d->grid_sly = 0;
	d = &graph.branches()[1]; d->embedded = true; d->grid_x = -1; d->grid_sly = 1;
	d = &graph.leaves()[0];   d->embedded = true; d->grid_x = -2; d->grid_sly = 0;
	d = &graph.leaves()[1];   d->embedded = true; d->grid_x = -2; d->grid_sly = 1;
	d = &graph.leaves()[2];   d->embedded = true; d->grid_x = -1; d->grid_sly = -1;

	auto embedder = WeakEmbedder{ graph };

	// layout sketch
	//   (L1). (B1).
	// (L0)--(B0)--(S0)
	//   (L2)`

	// Due to affinity, branch 2 will be placed below, not above.
	// We cannot place it far left next to L2, because it holds 4 leaves.
	// Therefore it must be placed one space further out.
	d = &graph.branches()[2];
	embedder.embed(*d);

	EXPECT_EQ(d->grid_x, 1);
	EXPECT_EQ(d->grid_sly, -1);
}

// The weak embedder must correctly lay out nodes in the general direction of
// its "principal direction" attribute. This is the basis for the "bend heuristic".
TEST(Embed, bend_direction)
{
	auto embedder = GridEmbedImpl{ 5 };
	Disk disk[5] = { Disk{ 0, 0, 0, 0, false } };
	embedder.putDiskAt(disk[0], { 0, 0 });

	embedder.principalDirection = Dir::RIGHT_DOWN;
	embedder.putDiskNear(disk[1], { 0, 0 }, GridEmbedImpl::Affinity::UP);
	EXPECT_EQ(disk[1].grid_x, 1);
	EXPECT_EQ(disk[1].grid_sly, -1);

	embedder.principalDirection = Dir::LEFT_DOWN;
	embedder.putDiskNear(disk[2], { 0, 0 }, GridEmbedImpl::Affinity::UP);
	EXPECT_EQ(disk[2].grid_x, 0);
	EXPECT_EQ(disk[2].grid_sly, -1);

	embedder.principalDirection = Dir::RIGHT;
	embedder.putDiskNear(disk[3], { 0, 0 }, GridEmbedImpl::Affinity::UP);
	EXPECT_EQ(disk[3].grid_x, 1);
	EXPECT_EQ(disk[3].grid_sly, 0);

	embedder.principalDirection = Dir::RIGHT_UP;
	embedder.putDiskNear(disk[4], { 0, 0 }, GridEmbedImpl::Affinity::UP);
	EXPECT_EQ(disk[4].grid_x, 0);
	EXPECT_EQ(disk[4].grid_sly, 1);
}

// The embedder must correctly evaluate the most appropriate bend direction.
TEST(Embed, determinate_principal)
{
	auto embedder = GridEmbedImpl{ 5 };
	Disk disk[5] = { Disk{ 0, 0, 0, 0, false } };
	embedder.putDiskAt(disk[0], { 0, 0 });
	embedder.putDiskAt(disk[1], { 1, 0 });

	// must be biased towards preserving previous principal dir
	Dir actual = embedder.determinePrincipal({ 1, 0 });
	EXPECT_EQ(actual, Dir::RIGHT);

	// must be biased in the general direction of the previous principal dir
	embedder.putDiskAt(disk[2], { 2, -2 });
	actual = embedder.determinePrincipal({ 1, 0 });
	EXPECT_EQ(actual, Dir::RIGHT_UP);

	// must be biased against blocked branches
	embedder.putDiskAt(disk[3], { 2, 0 });
	embedder.putDiskAt(disk[4], { 2, 1 });
	actual = embedder.determinePrincipal({ 1, 0 });
	EXPECT_EQ(actual, Dir::LEFT_UP);
}
