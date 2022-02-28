#include "enumerate.h"
#include <algorithm>
#include <cassert>

Enumerate::Enumerate(Embedder& fast, WholesaleEmbedder& reference, int minSize, int maxSize) noexcept
	: fast_(&fast), embedOrder_(Configuration::EmbedOrder::DEPTH_FIRST),
	reference_(&reference), minSize_(minSize), maxSize_(maxSize),
	current_(), lastSuccess_(true), output_(nullptr), stats_()
{
	assert(minSize >= 0);
	assert(minSize < maxSize);

	Lobster::Spine empty;
	empty.fill(Lobster::NO_BRANCH);
	current_ = Lobster(std::vector<Lobster::Spine>(minSize, empty));
}

void Enumerate::next()
{
	auto& spine = current_.spine();
	const auto NB = Lobster::NO_BRANCH;

	int i = spine.size() - 1; // spine index
	int j = 4; // branch index

	// reference-based skip: we do not evaluate the bigger lobsters after a fail
	if (!lastSuccess_) {
		// remove the last branch from the back
		while (i >= 0) {
			for (j = 4; j >= 0; j--) {
				if (spine[i][j] != NB) {
					spine[i][j] = NB;

					if (j > 0) {
						j--;
					}
					else {
						j = 4;
						i--;
					}
					goto addleaf;
				}
			}

			i--; // try previous spine vertex
		}
	}

addleaf:
	// add one leaf as far back as we can
	while (i >= 0) { // try to find a place on existing spine
		while ((j > 0) && (spine[i][j] == spine[i][j - 1]))
			j--; // skip over non-ordered branch configs (non-canonical)

		if (spine[i][j] < 5) { // we can increment here
			spine[i][j]++;

			if (isCanonicallyOriented(current_)) {
				return; // success
			}
			else {
				// reset counters for next increment
				i = spine.size() - 1;
				j = 4;
				continue;
			}
		}

		// increment overflow - carry to next "digit"
		for(int k = j; k < 5; k++)
			spine[i][k] = NB;

		j--;

		if (j < 0) { // try previous spine vertex
			i--;
			j = 4;
		}
	}

	// all possibilities iterated - enlarge spine
	Lobster::Spine empty = { NB, NB, NB, NB, NB };
	current_ = Lobster(std::vector<Lobster::Spine>(spine.size() + 1, empty));
}

bool Enumerate::test()
{
	return lastSuccess_ = test(current_);
}

bool Enumerate::test(const Lobster& lobster)
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

	return stat.success;
}

const Lobster& Enumerate::current() const noexcept
{
	return current_;
}

void Enumerate::setCurrent(Lobster lobster) noexcept
{
	current_ = std::move(lobster);
	lastSuccess_ = true;
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

bool Enumerate::isCanonicallyOriented(const Lobster& lobster) noexcept
{
	const auto& spine = lobster.spine();
	int head = 0;
	int tail = spine.size() - 1;

	for (; head < spine.size(); head++, tail--) {
		// head-heavy?
		if (std::ranges::lexicographical_compare(spine[tail], spine[head]))
			return true;

		// tail-heavy?
		if (std::ranges::lexicographical_compare(spine[head], spine[tail]))
			return false;
	}

	// palindrome
	return true;
}
