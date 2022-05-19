#include "enumerate.h"
#include "utility/util.h"
#include <algorithm>
#include <cassert>

Enumerate::Enumerate(Embedder& fast, WholesaleEmbedder& reference, int minSize, int maxSize) noexcept
	: fast_(&fast), embedOrder_(Configuration::EmbedOrder::DEPTH_FIRST),
	reference_(&reference), minSize_(minSize), maxSize_(maxSize),
	current_(), evaluation_(), output_(nullptr), csv_(nullptr), archive_(nullptr), stats_()
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
	if (!evaluation_.refSuccess) {
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

const Evaluation& Enumerate::test()
{
	return evaluation_ = test(current_);
}

Evaluation Enumerate::test(const Lobster& lobster)
{
	// *** run fast heuristic test ***

	// hardcoded because this is the only lobster heuristic option
	auto algorithm = Configuration::Algorithm::CLEVE;

	DiskGraph fastGraph = DiskGraph::fromLobster(lobster);
	Stat fastStat = embed(fastGraph, *fast_, algorithm, embedOrder_);

	// *** run reference test ***

	DiskGraph refGraph = DiskGraph::fromLobster(lobster);
	Stat refStat = embedDynamic(refGraph, *reference_);

	// debug sanity check: the reference implementation is strictly more accurate
	assert(refStat.success || !fastStat.success);

	// *** record statistics ***

	if (csv_) {
		csv_->write(fastStat);
		csv_->write(refStat);
	}
	else {
		stats_.push_back(fastStat);
		stats_.push_back(refStat);
	}

	if (archive_) {
		archive_->write(lobster, refStat.success);
	}

	// *** produce output if we are on the "line" between feasible/infeasible ***
	if (output_) {
		output_->ensureBatch();

		// TODO: an interesting instance leaves no space next to spines/branches
		if (evaluation_.fastSuccess && !fastStat.success)
			output_->write(evaluation_.fastResult, format("fast {} spines {} total", fastStat.spines, fastStat.size));

		if (evaluation_.refSuccess && !refStat.success)
			output_->write(evaluation_.refResult, format("reference {} spines {} total", refStat.spines, refStat.size));
	}

	return { fastStat.success, std::move(fastGraph), refStat.success, std::move(refGraph) };
}

const Lobster& Enumerate::current() const noexcept
{
	return current_;
}

void Enumerate::setCurrent(Lobster lobster) noexcept
{
	current_ = std::move(lobster);
	evaluation_.fastSuccess = true;
	evaluation_.refSuccess = true;
}

void Enumerate::setEmbedOrder(Configuration::EmbedOrder embedOrder) noexcept
{
	embedOrder_ = embedOrder;
}

void Enumerate::setOutput(Svg* output) noexcept
{
	output_ = output;
}

void Enumerate::setCsv(Csv* csv) noexcept
{
	csv_ = csv;
}

void Enumerate::setArchive(Archive* archive) noexcept
{
	archive_ = archive;
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
