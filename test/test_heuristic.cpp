// Embed heuristic unit tests

#include "gtest/gtest.h"
#include "heuristic.h"

namespace
{

// a simple easy example caterpillar: spine [0] with 3 leaves, spine [1] with 4 leaves
DiskGraph make_caterpillar()
{
	std::vector<Disk> disks(9);

	disks[0].id = 0;
	disks[0].depth = 0;
	disks[0].parent = nullptr;
	disks[0].nextSibling = &disks[1];
	disks[0].child = &disks[2];

	disks[1].id = 1;
	disks[1].depth = 0;
	disks[1].parent = nullptr;
	disks[1].prevSibling = &disks[0];
	disks[1].child = &disks[5];

	for (DiskId id = 2; id < 9; id++) {
		disks[id].parent = &disks[id < 5 ? 0 : 1];
		disks[id].depth = 1;
		disks[id].id = id;
	}

	for (DiskId id = 3; id < 9; id++)
		if (id != 5)
			disks[id].prevSibling = &disks[id - 1];

	for (DiskId id = 2; id < 8; id++)
		if (id != 4)
			disks[id].nextSibling = &disks[id + 1];

	return DiskGraph(move(disks));
}

}

TEST(Embed, embed_proper)
{
	auto graph = make_caterpillar();
	auto embedder = ProperEmbedder{};
	auto algorithm = Configuration::Algorithm::KLEMZ_NOELLENBURG_PRUTKIN;
	auto embedOrder = Configuration::EmbedOrder::DEPTH_FIRST;

	// execute
	Stat stat = embed(graph, embedder, algorithm, embedOrder);
	ASSERT_TRUE(stat.success);

	auto& disks = graph.disks();
	EXPECT_NEAR(disks[0].x, 0, 0.01);      EXPECT_NEAR(disks[0].y, 0, 0.01);
	EXPECT_NEAR(disks[1].x, 1, 0.01);      EXPECT_NEAR(disks[1].y, 0, 0.01);

	EXPECT_NEAR(disks[2].x, -1, 0.01);     EXPECT_NEAR(disks[2].y, 0, 0.01);
	EXPECT_NEAR(disks[3].x, -0.395, 0.01); EXPECT_NEAR(disks[3].y, -0.91889f, 0.01);
	EXPECT_NEAR(disks[4].x, -0.395, 0.01); EXPECT_NEAR(disks[4].y, 0.91889f, 0.01);

	EXPECT_NEAR(disks[5].x, .705, 0.01);   EXPECT_NEAR(disks[5].y, -0.95466f, 0.01);
	EXPECT_NEAR(disks[6].x, .705, 0.01);   EXPECT_NEAR(disks[6].y, 0.95466f, 0.01);
	EXPECT_NEAR(disks[7].x, 1.7605, 0.01); EXPECT_NEAR(disks[7].y, -0.6481f, 0.01);
	EXPECT_NEAR(disks[8].x, 1.7605, 0.01); EXPECT_NEAR(disks[8].y, 0.6481f, 0.01);
}

// a simple easy example lobster: spine [0] with 3 branches (2, 1, 1 leaves), spine [1] with 4 branches (first has 1 leaf)
DiskGraph make_lobster()
{
	std::vector<Disk> disks(14);

	disks[0].id = 0;
	disks[0].depth = 0;
	disks[0].parent = nullptr;
	disks[0].nextSibling = &disks[1];
	disks[0].child = &disks[2];

	disks[1].id = 1;
	disks[1].depth = 0;
	disks[1].parent = nullptr;
	disks[1].prevSibling = &disks[0];
	disks[1].child = &disks[5];

	for (DiskId id = 2; id < 14; id++) {
		if (id < 9) {
			disks[id].parent = &disks[id < 5 ? 0 : 1];
			disks[id].depth = 1;
			if (id < 6)
				disks[id].child = &disks[id == 2 ? 9 : id + 8];
		}
		else {
			disks[id].parent = &disks[id < 10 ? 2 : id - 8];
			disks[id].depth = 2;
		}

		disks[id].id = id;
	}

	for (DiskId id = 3; id < 11; id++)
		if (id != 5 && id != 9)
			disks[id].prevSibling = &disks[id - 1];

	for (DiskId id = 2; id < 10; id++)
		if (id != 4 && id != 8)
			disks[id].nextSibling = &disks[id + 1];

	return DiskGraph(move(disks));
}

TEST(Embed, embed_weak)
{
	auto graph = make_lobster();
	WeakEmbedder embedder;
	auto algorithm = Configuration::Algorithm::CLEVE;
	auto embedOrder = Configuration::EmbedOrder::DEPTH_FIRST;

	// execute, leaves first
	Stat stat = embed(graph, embedder, algorithm, embedOrder);
	ASSERT_TRUE(stat.success);

	auto& disks = graph.disks();
	// sbllblblsblbbb   spines: 0 8  branches: 1 4 6 9 11 12 13  leaves: 2 3 5 7 10
	EXPECT_NEAR(disks[0].x, 0, 0.01);     EXPECT_NEAR(disks[0].y, 0, 0.01);        EXPECT_EQ(disks[0].grid_x, 0);   EXPECT_EQ(disks[0].grid_sly, 0);
	EXPECT_NEAR(disks[1].x, 1, 0.01);     EXPECT_NEAR(disks[1].y, 0, 0.01);        EXPECT_EQ(disks[1].grid_x, 1);   EXPECT_EQ(disks[1].grid_sly, 0);

	EXPECT_NEAR(disks[2].x, -1, 0.01);    EXPECT_NEAR(disks[2].y, 0, 0.01);        EXPECT_EQ(disks[2].grid_x, -1);  EXPECT_EQ(disks[2].grid_sly, 0);
	EXPECT_NEAR(disks[3].x, -0.5, 0.01);  EXPECT_NEAR(disks[3].y, -0.866f, 0.01);  EXPECT_EQ(disks[3].grid_x, 0);   EXPECT_EQ(disks[3].grid_sly, -1);
	EXPECT_NEAR(disks[4].x, -0.5, 0.01);  EXPECT_NEAR(disks[4].y, 0.866f, 0.01);   EXPECT_EQ(disks[4].grid_x, -1);  EXPECT_EQ(disks[4].grid_sly, 1);

	EXPECT_NEAR(disks[5].x, .5, 0.01);    EXPECT_NEAR(disks[5].y, 0.866f, 0.01);   EXPECT_EQ(disks[5].grid_x, 0);   EXPECT_EQ(disks[5].grid_sly, 1);
	EXPECT_NEAR(disks[6].x, .5, 0.01);    EXPECT_NEAR(disks[6].y, -0.866f, 0.01);  EXPECT_EQ(disks[6].grid_x, 1);   EXPECT_EQ(disks[6].grid_sly, -1);
	EXPECT_NEAR(disks[7].x, 1.5, 0.01);   EXPECT_NEAR(disks[7].y, -0.866f, 0.01);  EXPECT_EQ(disks[7].grid_x, 2);   EXPECT_EQ(disks[7].grid_sly, -1);
	EXPECT_NEAR(disks[8].x, 1.5, 0.01);   EXPECT_NEAR(disks[8].y, 0.866f, 0.01);   EXPECT_EQ(disks[8].grid_x, 1);   EXPECT_EQ(disks[8].grid_sly, 1);

	EXPECT_NEAR(disks[9].x, -2, 0.01);    EXPECT_NEAR(disks[9].y, 0, 0.01);        EXPECT_EQ(disks[9].grid_x, -2);  EXPECT_EQ(disks[9].grid_sly, 0);
	EXPECT_NEAR(disks[10].x, -1.5, 0.01); EXPECT_NEAR(disks[10].y, 0.866f, 0.01);  EXPECT_EQ(disks[10].grid_x, -2); EXPECT_EQ(disks[10].grid_sly, 1);
	EXPECT_NEAR(disks[11].x, -1.5, 0.01); EXPECT_NEAR(disks[11].y, -0.866f, 0.01); EXPECT_EQ(disks[11].grid_x, -1); EXPECT_EQ(disks[11].grid_sly, -1);
	EXPECT_NEAR(disks[12].x, -1, 0.01);   EXPECT_NEAR(disks[12].y, 1.732f, 0.01);  EXPECT_EQ(disks[12].grid_x, -2); EXPECT_EQ(disks[12].grid_sly, 2);
	EXPECT_NEAR(disks[13].x, 0, 0.01);    EXPECT_NEAR(disks[13].y, 1.732f, 0.01);  EXPECT_EQ(disks[13].grid_x, -1); EXPECT_EQ(disks[13].grid_sly, 2);
}

TEST(Embed, embed_weak_breadthfirst)
{
	auto graph = make_lobster();
	WeakEmbedder embedder;
	auto algorithm = Configuration::Algorithm::CLEVE;
	auto embedOrder = Configuration::EmbedOrder::BREADTH_FIRST;

	// execute, spine first
	Stat stat = embed(graph, embedder, algorithm, embedOrder);
	ASSERT_TRUE(stat.success);

	auto& disks = graph.disks();
	// sbbbllllsbbbbl   spines: 0 8  branches: 1 2 3 9 10 11 12  leaves: 4 5 6 7 13
	EXPECT_NEAR(disks[0].x, 0, 0.01);     EXPECT_NEAR(disks[0].y, 0, 0.01);        EXPECT_EQ(disks[0].grid_x, 0);   EXPECT_EQ(disks[0].grid_sly, 0);
	EXPECT_NEAR(disks[1].x, 1, 0.01);     EXPECT_NEAR(disks[1].y, 0, 0.01);        EXPECT_EQ(disks[1].grid_x, 1);   EXPECT_EQ(disks[1].grid_sly, 0);

	EXPECT_NEAR(disks[2].x, -1, 0.01);    EXPECT_NEAR(disks[2].y, 0, 0.01);        EXPECT_EQ(disks[2].grid_x, -1);  EXPECT_EQ(disks[2].grid_sly, 0);
	EXPECT_NEAR(disks[3].x, -0.5, 0.01);  EXPECT_NEAR(disks[3].y, 0.866f, 0.01);   EXPECT_EQ(disks[3].grid_x, -1);  EXPECT_EQ(disks[3].grid_sly, 1);
	EXPECT_NEAR(disks[4].x, -0.5, 0.01);  EXPECT_NEAR(disks[4].y, -0.866f, 0.01);  EXPECT_EQ(disks[4].grid_x, 0);   EXPECT_EQ(disks[4].grid_sly, -1);

	EXPECT_NEAR(disks[5].x, .5, 0.01);    EXPECT_NEAR(disks[5].y, 0.866f, 0.01);   EXPECT_EQ(disks[5].grid_x, 0);   EXPECT_EQ(disks[5].grid_sly, 1);
	EXPECT_NEAR(disks[6].x, .5, 0.01);    EXPECT_NEAR(disks[6].y, -0.866f, 0.01);  EXPECT_EQ(disks[6].grid_x, 1);   EXPECT_EQ(disks[6].grid_sly, -1);
	EXPECT_NEAR(disks[7].x, 1.5, 0.01);   EXPECT_NEAR(disks[7].y, 0.866f, 0.01);   EXPECT_EQ(disks[7].grid_x, 1);   EXPECT_EQ(disks[7].grid_sly, 1);
	EXPECT_NEAR(disks[8].x, 1.5, 0.01);   EXPECT_NEAR(disks[8].y, -0.866f, 0.01);  EXPECT_EQ(disks[8].grid_x, 2);   EXPECT_EQ(disks[8].grid_sly, -1);

	EXPECT_NEAR(disks[9].x, -2, 0.01);    EXPECT_NEAR(disks[9].y, 0, 0.01);        EXPECT_EQ(disks[9].grid_x, -2);  EXPECT_EQ(disks[9].grid_sly, 0);
	EXPECT_NEAR(disks[10].x, -1.5, 0.01); EXPECT_NEAR(disks[10].y, 0.866f, 0.01);  EXPECT_EQ(disks[10].grid_x, -2); EXPECT_EQ(disks[10].grid_sly, 1);
	EXPECT_NEAR(disks[11].x, -1, 0.01);   EXPECT_NEAR(disks[11].y, 1.732f, 0.01);  EXPECT_EQ(disks[11].grid_x, -2); EXPECT_EQ(disks[11].grid_sly, 2);
	EXPECT_NEAR(disks[12].x, -1.5, 0.01); EXPECT_NEAR(disks[12].y, -0.866f, 0.01); EXPECT_EQ(disks[12].grid_x, -1); EXPECT_EQ(disks[12].grid_sly, -1);
	EXPECT_NEAR(disks[13].x, 0, 0.01);    EXPECT_NEAR(disks[13].y, 1.732f, 0.01);  EXPECT_EQ(disks[13].grid_x, -1); EXPECT_EQ(disks[13].grid_sly, 2);
}

// In this lobster, the branch on the second spine must be placed further out.
DiskGraph make_lobster_for_space_heuristic()
{
	std::vector<Disk> disks(11);

	// spine
	disks[0].id = 0;
	disks[0].parent = nullptr;
	disks[0].children = 3;
	disks[0].depth = 0;

	// branches
	for (DiskId id = 1; id < 4; id++) {
		disks[id].id = id;
		disks[id].parent = &disks[0];
		disks[id].depth = 1;
	}
	disks[1].children = 3;
	disks[3].children = 4;

	// leaves
	for (DiskId id = 4; id < 11; id++) {
		disks[id].id = id;
		disks[id].parent = &disks[id > 6 ? 3 : 1];
		disks[id].depth = 2;
	}

	return DiskGraph(move(disks));
}

// The weak embedder must observe the "space heuristic", i.e. place the next
// branch with enough leftover room to allow for all child vertices.
TEST(Embed, space_heuristic)
{
	auto graph = make_lobster_for_space_heuristic();

	// fix embedding until we need to apply the space heuristic
	auto& disks = graph.disks();
	Disk* d;
	d = &disks[0]; d->embedded = true; d->grid_x = 0; d->grid_sly = 0;
	d = &disks[1]; d->embedded = true; d->grid_x = -1; d->grid_sly = 0;
	d = &disks[2]; d->embedded = true; d->grid_x = -1; d->grid_sly = 1;
	d = &disks[4]; d->embedded = true; d->grid_x = -2; d->grid_sly = 0;
	d = &disks[5]; d->embedded = true; d->grid_x = -2; d->grid_sly = 1;
	d = &disks[6]; d->embedded = true; d->grid_x = -1; d->grid_sly = -1;

	WeakEmbedder embedder;
	embedder.setGraph(graph);

	// layout sketch
	//   (L1). (B1).
	// (L0)--(B0)--(S0)
	//   (L2)`

	// Due to affinity, branch 2 will be placed below, not above.
	// We cannot place it far left next to L2, because it holds 4 leaves.
	// Therefore it must be placed one space further out.
	d = &disks[3];
	embedder.embed(*d);

	EXPECT_EQ(d->grid_x, 1);
	EXPECT_EQ(d->grid_sly, -1);
}

// The weak embedder must correctly lay out nodes in the general direction of
// its "principal direction" attribute. This is the basis for the "bend heuristic".
TEST(Embed, bend_direction)
{
	auto embedder = GridEmbedImpl{ 5 };
	Disk disk[5];
	for (int i = 0; i < 5; i++) {
		disk[i].depth = 0;
		disk[i].children = 0;
	}
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
	Disk disk[5];
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
