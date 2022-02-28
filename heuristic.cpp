#include "heuristic.h"
#include "utility/util.h"
#include <algorithm>
#include <cassert>

/**
 * Cosmetic positioning for disks which cannot be placed by the algorithm.
 */
const float Y_FAIL = 2.2f;

ProperEmbedder::ProperEmbedder() noexcept :
	spine_{ 0, 0 },
	forward_{ 1.f, 0 },
	lastUp_{ -10, 1 },
	lastDown_{ -10, -1 },
	lastSpine_{ -1, 0 },
	leafUp_{ false },
	gap_{ 0.1f },
	beforeFirstSpine_{ true },
	atFirstSpine_{ false },
	beforeFirstLeaf_{ true }
{
}

void ProperEmbedder::setGap(float gap) noexcept
{
	gap_ = gap;
}

void ProperEmbedder::embed(Disk& disk)
{
	if (0 == disk.depth)
		return embedSpine(disk);

	if (1 == disk.depth)
		return embedLeaf(disk);

	else
		throw std::exception("proper embedder can not embed graphs deeper than caterpillars");
}

void ProperEmbedder::setGraph(DiskGraph& graph) noexcept
{
	// This embedder does not require graph knowledge.
}

void ProperEmbedder::embedSpine(Disk& disk) noexcept
{
	if (beforeFirstSpine_) {
		beforeFirstSpine_ = false;
		disk.x = spine_.x;
		disk.y = spine_.y;
		return;
	}

	// determine the bisector of the free space ahead as new "forward"
	const auto hypotheticalUp = findLeafPosition(lastUp_);
	const auto hypotheticalDown = findLeafPosition(lastDown_);
	const auto forward1 = (hypotheticalUp + hypotheticalDown - spine_ - spine_).unit();
	const auto forward2 = forward1 * -1;
	forward_ = distance(forward_, forward1) < distance(forward_, forward2) ? forward1 : forward2;

	// move forward
	lastSpine_ = spine_;
	spine_ += forward_;
	beforeFirstLeaf_ = false;

	disk.x = spine_.x;
	disk.y = spine_.y;
	disk.failure = distance(spine_, lastUp_) < 1 + gap_; // is there enough room for the spine?
}

void ProperEmbedder::embedLeaf(Disk& disk) noexcept
{
	assert(!beforeFirstSpine_);

	if (beforeFirstLeaf_) {
		beforeFirstLeaf_ = false;
		disk.x = -1;
		disk.y = 0;
		return;
	}

	Vec2& lastLeaf = leafUp_ ? lastUp_ : lastDown_;
	Vec2 leafPosition = findLeafPosition(lastLeaf);

	// leaf wrap-around collision due to too many leaves
	if (distance(leafPosition, leafUp_ ? lastDown_ : lastUp_) < 1 + gap_) {
		leafPosition.y += Y_FAIL;
		disk.x = leafPosition.x;
		disk.y = leafPosition.y;
		disk.failure = true;
		return;
	}

	leafUp_ = !leafUp_; // alternate placement
	lastLeaf = leafPosition;

	disk.x = leafPosition.x;
	disk.y = leafPosition.y;
}

Vec2 ProperEmbedder::findLeafPosition(Vec2 constraint) noexcept
{
	Vec2 leafPosition = triangulate(spine_, 1, constraint, 1 + gap_, forward_, gap_ * 0.01f);

	// If the position under the argument constraint is too close to the last spine,
	// try again using the last spine as a constraint.
	if (distance(lastSpine_, leafPosition) < 1 + gap_) {
		const Vec2 hint = leafPosition - lastSpine_;
		leafPosition = triangulate(spine_, 1, lastSpine_, 1 + gap_, hint, gap_ * 0.01f);
	}

	return leafPosition;
}


GridEmbedImpl::GridEmbedImpl(size_t size) noexcept
	: principalDirection(Dir::RIGHT), grid_(size)
{
}

const Grid& GridEmbedImpl::grid() const noexcept
{
	return grid_;
}

GridEmbedImpl::Affinity GridEmbedImpl::determineAffinity(Coord center) const noexcept
{
	// affinity is based on the available free space in the vicinity
	Coord upperArea[] = {
		grid_.step(center, principalDirection, Rel::BACK_UP),
		grid_.step(grid_.step(center, principalDirection, Rel::BACK_UP), principalDirection, Rel::BACK),
		grid_.step(grid_.step(center, principalDirection, Rel::BACK_UP), principalDirection, Rel::BACK_UP),
		grid_.step(grid_.step(center, principalDirection, Rel::BACK_UP), principalDirection, Rel::FWD_UP),
		grid_.step(center, principalDirection, Rel::FWD_UP),
		grid_.step(grid_.step(center, principalDirection, Rel::FWD_UP), principalDirection, Rel::FWD_UP),
		grid_.step(grid_.step(center, principalDirection, Rel::FWD_UP), principalDirection, Rel::FORWARD)
	};

	Coord lowerArea[] = {
		grid_.step(center, principalDirection, Rel::BACK_DOWN),
		grid_.step(grid_.step(center, principalDirection, Rel::BACK_DOWN), principalDirection, Rel::BACK),
		grid_.step(grid_.step(center, principalDirection, Rel::BACK_DOWN), principalDirection, Rel::BACK_DOWN),
		grid_.step(grid_.step(center, principalDirection, Rel::BACK_DOWN), principalDirection, Rel::FWD_DOWN),
		grid_.step(center, principalDirection, Rel::FWD_DOWN),
		grid_.step(grid_.step(center, principalDirection, Rel::FWD_DOWN), principalDirection, Rel::FWD_DOWN),
		grid_.step(grid_.step(center, principalDirection, Rel::FWD_DOWN), principalDirection, Rel::FORWARD)
	};

	int upperWeight = 0;
	int lowerWeight = 0;

	for (int i = 0; i < 7; i++) {
		upperWeight += nullptr != grid_.at(upperArea[i]);
		lowerWeight += nullptr != grid_.at(lowerArea[i]);
	}

	return lowerWeight < upperWeight ? Affinity::DOWN : Affinity::UP;
}

Dir GridEmbedImpl::determinePrincipal(Coord tip) const noexcept
{
	// bias candidates towards preserving current principal
	Dir dir = principalDirection;
	Dir candidates[6] = { dir + Rel::FORWARD, dir + Rel::FWD_DOWN, dir + Rel::FWD_UP,
		dir + Rel::BACK_DOWN, dir + Rel::BACK_UP, dir + Rel::BACK };

	Dir principal; // best candidate
	int bestValue = 200; // higher value = more blocked spaces, less desirable

	for (int i = 0; i < 6; i++) {
		Coord center = grid_.step(tip, candidates[i], Rel::FORWARD);

		// affinity is based on the available free space in the vicinity
		Coord branchPoints[] = {
			grid_.step(center, principalDirection, Rel::FORWARD),
			grid_.step(center, principalDirection, Rel::BACK),
			grid_.step(center, principalDirection, Rel::BACK_UP),
			grid_.step(center, principalDirection, Rel::FWD_UP),
			grid_.step(center, principalDirection, Rel::BACK_DOWN),
			grid_.step(center, principalDirection, Rel::FWD_DOWN)
		};

		Coord leafPoints[] = {
			grid_.step(grid_.step(center, principalDirection, Rel::FORWARD), principalDirection, Rel::FORWARD),
			grid_.step(grid_.step(center, principalDirection, Rel::FORWARD), principalDirection, Rel::FWD_UP),
			grid_.step(grid_.step(center, principalDirection, Rel::FORWARD), principalDirection, Rel::FWD_DOWN),
			grid_.step(grid_.step(center, principalDirection, Rel::FWD_UP), principalDirection, Rel::FWD_UP),
			grid_.step(grid_.step(center, principalDirection, Rel::FWD_DOWN), principalDirection, Rel::FWD_DOWN),
			grid_.step(grid_.step(center, principalDirection, Rel::BACK_UP), principalDirection, Rel::FWD_UP),
			grid_.step(grid_.step(center, principalDirection, Rel::BACK_UP), principalDirection, Rel::BACK_UP),
			grid_.step(grid_.step(center, principalDirection, Rel::BACK_DOWN), principalDirection, Rel::FWD_DOWN),
			grid_.step(grid_.step(center, principalDirection, Rel::BACK_DOWN), principalDirection, Rel::BACK_DOWN),
			grid_.step(grid_.step(center, principalDirection, Rel::BACK), principalDirection, Rel::BACK),
			grid_.step(grid_.step(center, principalDirection, Rel::BACK), principalDirection, Rel::BACK_UP),
			grid_.step(grid_.step(center, principalDirection, Rel::BACK), principalDirection, Rel::BACK_DOWN)
		};

		int value = 0;

		if (grid_.at(center))
			value += 100;

		for (int j = 0; j < 6; j++) {
			if (grid_.at(branchPoints[j]))
				value += 2;
		}

		for (int j = 0; j < 12; j++) {
			if (grid_.at(leafPoints[j]))
				value += 1;
		}

		if (value < bestValue) {
			bestValue = value;
			principal = candidates[i];
		}
	}

	return principal;
}

int GridEmbedImpl::countFreeNeighbors(Coord center) const noexcept
{
	Coord neighbors[] = {
		grid_.step(center, principalDirection, Rel::BACK),
		grid_.step(center, principalDirection, Rel::BACK_UP),
		grid_.step(center, principalDirection, Rel::BACK_DOWN),
		grid_.step(center, principalDirection, Rel::FWD_UP),
		grid_.step(center, principalDirection, Rel::FWD_DOWN),
		grid_.step(center, principalDirection, Rel::FORWARD)
	};

	return std::count_if(neighbors, neighbors + 6,
		[this](Coord c) { return !grid_.at(c); });
}

void GridEmbedImpl::putDiskNear(Disk& disk, Coord coord, Affinity affinity) noexcept
{
	static const Rel upCandidates[6] = { Rel::BACK, Rel::BACK_UP, Rel::FWD_UP, Rel::FORWARD, Rel::FWD_DOWN, Rel::BACK_DOWN };
	static const Rel downCandidates[6] = { Rel::BACK, Rel::BACK_DOWN, Rel::FWD_DOWN, Rel::FORWARD, Rel::FWD_UP, Rel::BACK_UP };

	const Rel* candidates, * end;

	if (0 == disk.depth) {
		static const Rel spineRel = Rel::FORWARD;
		candidates = &spineRel;
		end = candidates + 1;
	}
	else {
		if (Affinity::UP == affinity)
			candidates = upCandidates;
		else
			candidates = downCandidates;

		end = candidates + 6;
	}

	for (; candidates != end; ++candidates) {
		Coord target = grid_.step(coord, principalDirection, *candidates);

		if (!grid_.at(target) &&
			// space heuristic: we must leave space for leaves
			countFreeNeighbors(target) >= disk.children) {

			putDiskAt(disk, target);
			return;
		}
	}

	// no free coordinate found
	disk.failure = true;
}

void GridEmbedImpl::putDiskAt(Disk& disk, Coord coord) noexcept
{
	grid_.put(coord, disk);
	disk.grid_x = coord.x;
	disk.grid_sly = coord.sly;
	Vec2 diskVec = grid_.vec(coord);
	disk.x = diskVec.x;
	disk.y = diskVec.y;
}


WeakEmbedder::WeakEmbedder() noexcept :
	graph_(nullptr), impl_(0)
{
}

void WeakEmbedder::embed(Disk& disk)
{
	assert(graph_);

	if (0 == disk.depth)
		return embedSpine(disk);

	if (1 == disk.depth || 2 == disk.depth)
		return embedBranchOrLeaf(disk);

	else
		throw std::exception("weak embedder can not embed graphs deeper than lobsters");
}

void WeakEmbedder::setGraph(DiskGraph& graph) noexcept
{
	graph_ = &graph;
	impl_ = GridEmbedImpl(graph.size());

	// sync grid to graph state
	for (Disk& disk : graph.spines()) {
		if (disk.embedded)
			impl_.putDiskAt(disk, { disk.grid_x, disk.grid_sly });
	}
	for (Disk& disk : graph.branches()) {
		if (disk.embedded)
			impl_.putDiskAt(disk, { disk.grid_x, disk.grid_sly });
	}
	for (Disk& disk : graph.leaves()) {
		if (disk.embedded)
			impl_.putDiskAt(disk, { disk.grid_x, disk.grid_sly });
	}
}

void WeakEmbedder::embedSpine(Disk& disk) noexcept
{
	Coord coord{ 0, 0 };

	if (-1 != disk.parent)
	{
		const Disk* parent = graph_->findDisk(disk.parent);
		assert(parent);
		Coord parentCoord{ parent->grid_x, parent->grid_sly };

		// bend heuristic
		impl_.principalDirection = impl_.determinePrincipal(parentCoord);

		coord = impl_.grid().step(parentCoord, impl_.principalDirection, Rel::FORWARD);
	}

	if (impl_.grid().at(coord)) {
		disk.failure = true;
		trace("FAIL spine id {}", disk.id);
	}
	else {
		impl_.putDiskAt(disk, coord);
		trace("Embed spine id {} at ({}/{})", disk.id, disk.x, disk.y);
	}
}

void WeakEmbedder::embedBranchOrLeaf(Disk& disk) noexcept
{
	assert(NODISK != disk.parent); // branches and leaves always have parents

	const Disk* parent = graph_->findDisk(disk.parent);
	assert(parent);
	Coord parentCoord{ parent->grid_x, parent->grid_sly };
	GridEmbedImpl::Affinity affinity = impl_.determineAffinity(parentCoord); // whether to place disk high or low

	impl_.putDiskNear(disk, parentCoord, affinity);

	if (disk.failure) {
		trace(1 == disk.depth ? "FAIL branch id {}" : "FAIL leaf id {}", disk.id);
	}
	else {
		trace(1 == disk.depth ? "Embed branch id {} at ({}/{})" : "Embed leaf id {} at ({}/{})", disk.id, disk.x, disk.y);
	}
}
