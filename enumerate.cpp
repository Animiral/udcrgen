#include "enumerate.h"
#include <cassert>

Enumerate::Enumerate(Embedder& fast, WholesaleEmbedder& reference, int minSize, int maxSize) noexcept
	: fast_(&fast), embedOrder_(Configuration::EmbedOrder::DEPTH_FIRST),
	reference_(&reference), minSize_(minSize), maxSize_(maxSize),
	current_(), output_(nullptr), stats_()
{
	assert(minSize >= 0);
	assert(minSize < maxSize);

	Lobster::Spine empty;
	empty.fill(Lobster::NO_BRANCH);
	current_ = Lobster(std::vector<Lobster::Spine>(minSize, empty));
}

void Enumerate::next()
{
	// TODO
}

void Enumerate::test()
{
	test(current_);
}

void Enumerate::test(const Lobster& lobster)
{
	Stat stat;
	stat.size = lobster.countVertices();
	stat.spines = lobster.countSpine();

	// *** run fast heuristic test ***

	// hardcoded because this is the only lobster heuristic option
	stat.algorithm = Configuration::Algorithm::CLEVE;

	DiskGraph graph = DiskGraph::fromLobster(lobster);
	stat.success = embed(graph, *fast_, embedOrder_);
	stat.duration = stat.duration.zero(); // TODO: implement timer

	stats_.push_back(stat);

	// *** run reference test ***
	stat.algorithm = Configuration::Algorithm::DYNAMIC_PROGRAM;

	graph = DiskGraph::fromLobster(lobster); // reset
	stat.success = embedDynamic(graph, *reference_);
	stat.duration = stat.duration.zero(); // TODO: implement timer

	stats_.push_back(stat);
}

const Lobster& Enumerate::current() const noexcept
{
	return current_;
}

void Enumerate::setEmbedOrder(Configuration::EmbedOrder embedOrder) noexcept
{
	embedOrder_ = embedOrder;
}

void Enumerate::setOutput(Svg& output)
{
	output_ = &output;
}

const std::vector<Stat>& Enumerate::stats() const noexcept
{
	return stats_;
}

void Enumerate::run()
{
	while (current_.countSpine() < maxSize_) {
		test();
		next();
	}
}
