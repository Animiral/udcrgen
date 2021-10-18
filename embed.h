#pragma once

#include <utility>
#include "utility/graph.h"
#include "utility/grid.h"
#include "utility/geometry.h"
#include "config.h"

/**
 * The graph classes that the algorithms in this reportoire can differentiate.
 */
enum class GraphClass { CATERPILLAR, LOBSTER, OTHER };

/**
 * Take the raw input graph in the form of an edge list and prepare it for processing by the
 * embedding algorithm.
 *
 * The most important aspect is that we recognize which of the vertices constitute the
 * spine of the graph.
 *
 * Reorder all graph vertices with spines in front.
 * Then place branches in the order of
 * the spine vertex that is their parent.
 * Place leaves on a branch immediately following the branch vertex.
 *
 * @return the prepared graph and a type classification.
 */
std::pair<DiskGraph, GraphClass> classify(EdgeList input);

/**
 * An Embedder can embed single disks on the 2D plane.
 */
class Embedder
{

public:

	/**
	 * Place the next disk.
	 */
	virtual void embed(Disk& disk) = 0;

};

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
	Grid grid_;

	void embedSpine(Disk& disk) noexcept;
	void embedBranchOrLeaf(Disk& disk) noexcept;

	/**
	 * Which way around we'll attempt to find a free slot.
	 */
	enum class Affinity { UP = -1, DOWN = 1 };

	void putDiskNear(Disk& disk, Coord coord, Affinity affinity) noexcept;
	void putDiskAt(Disk& disk, Coord coord) noexcept;
	Affinity determineAffinity(Coord center) noexcept;

};

/**
 * Apply an embedding to the graph using the given embedding strategy.
 */
void embed(DiskGraph& graph, Embedder& embedder, Configuration::EmbedOrder embedOrder);
