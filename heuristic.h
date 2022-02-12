// Embedding algorithms based on fast heuristics

#pragma once

#include "embed.h"

/**
 * The proper embedder provides the state and operations to run the unit disk
 * contact graph embedding algorithm based on the Klemz et al. paper.
 *
 * It exclusively handles caterpillar graphs and will reject deeper disks
 * with an exception.
 */
class ProperEmbedder : public Embedder
{

public:

	/**
	 * Construct the embedder.
	 */
	explicit ProperEmbedder() noexcept;

	/**
	 * Configure the gap value.
	 */
	void setGap(float gap) noexcept;

	/**
	 * Place the next disk.
	 *
	 * For spines, the position is chosen along the bisector
	 * of the free space ahead, in contact with the current spine.
	 *
	 * For leaves, the position is chosen to be in contact with the
	 * current spine and to respect the gap to other already embedded disks.
	 */
	virtual void embed(Disk& disk) override;

private:

	Vec2 spine_; //!< embedding position for the current spine piece
	Vec2 forward_; //!< general direction for next spine
	Vec2 lastUp_; //!< placement of previous up leaf
	Vec2 lastDown_; //!< placement of previous down leaf
	Vec2 lastSpine_; //!< placement of previous spine
	bool leafUp_; //!< where to place the next leaf (alternate)
	float gap_; //!< minimum distance between two disks not in contact
	bool beforeFirstSpine_; //!< true before first spine placement
	bool atFirstSpine_; //!< true after first spine placement, before 2nd
	bool beforeFirstLeaf_; //!< true before first leaf placement

	/**
	 * Propose a position to possibly place a leaf as close as possible
	 * to the given constraining disk.
	 *
	 * This proposal is used to calculate possible positions for placement
	 * of new leaves and also new spines.
	 *
	 * @param constraint position of a pre-existing disk (e.g. a previous leaf)
	 */
	Vec2 findLeafPosition(Vec2 constraint) noexcept;

	void embedSpine(Disk& disk) noexcept;
	void embedLeaf(Disk& disk) noexcept;
};

/**
 * This class embeds disks on a triangular grid. It provides the implementation
 * details for the WeakEmbedder with its heuristics.
 */
class GridEmbedImpl
{

public:

	/**
	 * @brief Construct the grid embedder.
	 *
	 * @param size number of expected disks for grid capacity
	 */
	explicit GridEmbedImpl(size_t size) noexcept;

	/**
	 * @brief Which way around we'll attempt to find a free slot.
	 *
	 * The naming refers to the affinity under the default principal
	 * (right) direction. More precisely, UP = counter-clockwise,
	 * DOWN = clockwise.
	 */
	enum class Affinity { UP = -1, DOWN = 1 };

	Dir principalDirection;

	const Grid& grid() const noexcept;
	Affinity determineAffinity(Coord center) const noexcept;

	/**
	 * @brief calculate the most viable principal direction around the tip.
	 *
	 * @param tip last spine vertex location - next should be placed adjacent
	 * @return the direction with the most free space
	 */
	Dir determinePrincipal(Coord tip) const noexcept;
	int countFreeNeighbors(Coord center) const noexcept;

	void putDiskNear(Disk& disk, Coord coord, Affinity affinity) noexcept;
	void putDiskAt(Disk& disk, Coord coord) noexcept;

private:

	Grid grid_;

};

/**
 * The weak embedder provides the state and operations to run the unit disk
 * contact graph embedding algorithm based on the Cleve paper.
 */
class WeakEmbedder : public Embedder
{

public:

	/**
	 * Construct the embedder.
	 */
	explicit WeakEmbedder(DiskGraph& graph) noexcept;

	/**
	 * Place the next disk.
	 */
	virtual void embed(Disk& disk) override;

private:

	DiskGraph* graph_;
	GridEmbedImpl impl_;

	void embedSpine(Disk& disk) noexcept;
	void embedBranchOrLeaf(Disk& disk) noexcept;

};
