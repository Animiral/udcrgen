#pragma once

#include <utility>
#include "graph.h"
#include "geometry.h"

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
 * Stores the result of an attempt to embed a single disk.
 */
struct EmbedResult
{
	Vec2 position; //!< placement for the new disk
	bool failure; //!< true if the position adheres to disk contact constraints, false otherwise

	void applyTo(Disk& disk) noexcept;
};

/**
 * An Embedder can embed single disks on the 2D plane.
 */
class Embedder
{

public:

	/**
	 * Place the next spine.
	 */
	virtual EmbedResult spine() noexcept = 0;

	/**
	 * Place the next branch.
	 */
	virtual EmbedResult branch() noexcept = 0;

	/**
	 * Place the next leaf.
	 */
	virtual EmbedResult leaf() noexcept = 0;

};

/**
 * The proper embedder provides the state and operations to run the unit disk
 * contact graph embedding algorithm based on the Klemz et al. paper.
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
	 * Place the next spine.
	 *
	 * The position is chosen along the bisector of the free space ahead,
	 * in contact with the current spine.
	 */
	virtual EmbedResult spine() noexcept override;

	/**
	 * Place the next branch.
	 */
	virtual EmbedResult branch() noexcept override;

	/**
	 * Place the next leaf.
	 * 
	 * The position is chosen to be in contact with the current spine and
	 * to respect the gap to other already embedded disks.
	 */
	virtual EmbedResult leaf() noexcept override;

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
	explicit WeakEmbedder(int spineCount) noexcept;

	/**
	 * Place the next spine.
	 */
	virtual EmbedResult spine() noexcept override;

	/**
	 * Place the next branch.
	 */
	virtual EmbedResult branch() noexcept override;

	/**
	 * Place the next leaf.
	 */
	virtual EmbedResult leaf() noexcept override;

private:

	/**
	 * Position names with index offsets relative to any locality.
	 */
	enum class Rel { SPINE = 12, BEHIND = -5, UP = -6, DOWN = 1, FWD_UP = -1, FWD_DOWN = 6, FRONT = 5 };

	/**
	 * Which way around we'll attempt to find a free slot.
	 */
	enum class Affinity { UP = -1, DOWN = 1 };

	bool zone_[25]; //!< markers for used space
	int locality_; //!< zone index of current branch
	Affinity affinity_; //!< current affinity
	int spineIndex_; //!< offset of zone coordinates
	int spineCount_; //!< enables reserving space for all spines exactly

	/**
	 * Determine a zone index in the current locality that we may use
	 * to place another disk.
	 */
	int findFreePosition(int locality, Affinity affinity) noexcept;

	/**
	 * Turn the given position in the current zone into embedding coordinates
	 * under consideration of the current spine offset.
	 */
	Vec2 getCoords(int position) noexcept;

};

/**
 * Apply an embedding to the graph using the given embedding strategy.
 */
void embed(DiskGraph& graph, Embedder& embedder);
