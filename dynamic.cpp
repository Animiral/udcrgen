#include "dynamic.h"
#include <algorithm>
#include "utility/util.h"

Fundament::Fundament() noexcept = default;

Fundament::Fundament(const Fundament& rhs) noexcept = default;

Fundament::Fundament(const Grid& grid, Coord spineHead) noexcept
{
	for (int x = -2; x <= 2; x++) {
		for (int sly = -x-2; sly <= 2-x; sly++) {
			Coord c{ spineHead.x + x, spineHead.sly + sly };
			bool blocked = grid.at(c) != nullptr;
			int n = index({ x,sly });
			mask.set(n, blocked);
		}
	}
}

bool Fundament::operator==(const Fundament& rhs) const noexcept = default;

int Fundament::index(Coord c) noexcept
{
	if (c.x >= -2 && c.x <= 2 && c.sly + c.x >= -2 && c.sly + c.x <= 2) {
		return (c.sly + c.x + 2) * 5 + (c.x + 2);
	}
	else {
		return -1;
	}
}

Coord Fundament::at(int bit)
{
	assert(bit >= 0);
	assert(bit < 25);

	int x = bit % 5 - 2;
	int sly = bit / 5 - x - 2;
	return { x, sly };
}

bool Fundament::blocked(Coord c) const noexcept
{
	return mask.test(index(c));
}

Fundament Fundament::reachable(Coord from, int steps) const noexcept
{
	Fundament result;
	result.mask.set(); // block everything
	result.mask.set(index(from), false);

	Coord near[] = { { -1, 0 }, { -1, 1 }, {0, 1}, {1, 0}, {1, -1}, {0, -1} };
	Coord next[6];

	for (int step = 0; step < steps; step++) {
		Fundament mid = result;

		for (int bit = 0; bit < 25; bit++) {
			if (!result.mask.test(bit)) {
				// expand 1 step from here
				Coord e = at(bit);
				std::transform(near, near + 6, next,
					[e](Coord n) { return Coord{ e.x + n.x, e.sly + n.sly }; });

				for (Coord n : next) {
					if (index(n) != -1 && !blocked(n))
						mid.mask.set(index(n), false);
				}
			}
		}

		result = mid;
	}

	result.mask.set(index(from), true); // from is always blocked
	return result;
}

Fundament Fundament::reachableBySpine(Coord from) const noexcept
{
	Fundament result;
	result.mask.set(); // block everything

	Coord tos[] = { {from.x, from.sly + 1}, { from.x + 1, from.sly }, { from.x + 1, from.sly - 1 } };
	for (Coord to : tos) {
		int bit = index(to);
		if (bit >= 0 && !mask.test(bit))
			result.mask.set(bit, false);
	}

	return result;
}

#include <iostream>

[[maybe_unused]]
void Fundament::print() const
{
	for (int sly = 4; sly > 0; sly--) {
		if(sly % 2) std::cout << " ";
		for(int i = 0; i < sly/2; i++) std::cout << "  ";

		for (int x = -2; x < 2 - sly; x++) {
			int n = (sly + x + 2) * 5 + (x + 2);
			std::cout << (mask.test(n) ? "O " : "- ");
		}

		std::cout << "\n";
	}

	for (int sly = 0; sly >= -4; sly--) {
		if (sly % 2) std::cout << " ";
		for (int i = 0; i < -sly / 2; i++) std::cout << "  ";

		for (int x = -2-sly; x < 2; x++) {
			int n = (sly + x + 2) * 5 + (x + 2);
			std::cout << (mask.test(n) ? "O " : "- ");
		}

		std::cout << "\n";
	}
}

bool Signature::operator==(const Signature& rhs) const noexcept = default;

DynamicProblem::DynamicProblem(InputDisks& input)
	: solution_(input.size()),
	spineHead_(),
	branchHead_(),
	input_(&input),
	depth_(0)
{
}

DynamicProblem::DynamicProblem(InputDisks& input, const Grid& solution, Coord spineHead, Coord branchHead)
	: solution_(solution),
	spineHead_(spineHead),
	branchHead_(branchHead),
	input_(&input),
	depth_(static_cast<int>(solution.size()))
{
}

std::vector<DynamicProblem> DynamicProblem::subproblems() const
{
	Disk* disk = &input_->at(depth_);

	// always place the first disk at (0,0)
	if (0 == depth_) {
		Grid solution(input_->size());
		solution.put({ 0, 0 }, *disk);
		Coord spineHead = { 0, 0 };
		return { DynamicProblem(*input_, solution, spineHead, {}) };
	}

	// place disk next to the appropriate head
	Coord head;
	if (2 == disk->depth) {
		head = branchHead_;
	}
	else if (disk->depth < 2) {
		head = spineHead_;
	}
	else {
		throw std::exception("dynamic program can not embed graphs deeper than lobsters");
	}

	// create map of blocked spaces
	Fundament fundament(solution_, spineHead_);

	// we have up to 6 choices to place the upcoming disk
	Rel rels[6] = { Rel::BACK, Rel::BACK_UP, Rel::BACK_DOWN, Rel::FORWARD, Rel::FWD_UP, Rel::FWD_DOWN };
	Coord candidates[6];
	std::transform(rels, rels + 6, candidates, [this, head](Rel rel)
		{ return solution_.step(head, Dir::RIGHT, rel); });

	Coord* begin = candidates;
	Coord* end = candidates + 6;

	// spine is x-monotone -> limit to forward placement options
	if (0 == disk->depth) {
		begin = candidates + 3;
	}

	// do not consider blocked candidate spaces
	end = std::remove_if(begin, end, [this, fundament](Coord c)
		{ return fundament.blocked({ c.x - spineHead_.x, c.sly - spineHead_.sly }); });

	std::vector<DynamicProblem> subproblems;

	for (auto it = begin; it != end; ++it) {
		Grid solution = solution_;
		solution.put(*it, *disk);
		Coord spineHead = 0 == disk->depth ? *it : spineHead_;
		Coord branchHead = 1 == disk->depth ? *it : branchHead_;
		subproblems.push_back({ *input_, solution, spineHead, branchHead });
	}

	return subproblems;
}

void DynamicProblem::setSolution(const Grid& solution, Coord spineHead, Coord branchHead)
{
	solution_ = solution;
	spineHead_ = spineHead;
	branchHead_ = branchHead;
	depth_ = static_cast<int>(solution.size());
}

const Grid& DynamicProblem::solution() const noexcept
{
	return solution_;
}

Coord DynamicProblem::spineHead() const noexcept
{
	return spineHead_;
}

Coord DynamicProblem::branchHead() const noexcept
{
	return branchHead_;
}

int DynamicProblem::depth() const noexcept
{
	return depth_;
}

Signature DynamicProblem::signature() const noexcept
{
	Fundament fundament = Fundament(solution_, spineHead_);
	Coord head{ 0, 0 };

	// consider branch head only if it is relevant at this point (upcoming leaf)
	Disk* upcomingDisk = depth_ < input_->size() ? &(*input_)[depth_] : nullptr;
	if (upcomingDisk != nullptr && 2 == upcomingDisk->depth) {
		head = { branchHead_.x - spineHead_.x, branchHead_.sly - spineHead_.sly };
	}

	// disregard unreachable spaces
	fundament = reachableEventually(fundament, head, *input_, depth_);

	// derive transformed alternative from fundament by mirroring, choose one.
	std::bitset<25> mirrored = fundament.mask;

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4 - x; y++) {
			int upper = 5 + x * 6 + y * 5;
			int lower = 1 + x * 6 + y;

			// swap one bit
			bool temp = mirrored[upper];
			mirrored[upper] = mirrored[lower];
			mirrored[lower] = temp;
		}
	}

	// the lesser fundament, viewed as ulong, is the normal one
	if (mirrored.to_ulong() < fundament.mask.to_ulong()) {
		fundament.mask = mirrored;
		head.x += head.sly;
		head.sly = -head.sly;
	}

	return Signature{ depth_, fundament, head };
}


Fundament reachableEventually(Fundament base, Coord head, const InputDisks& input, int depth) noexcept
{
	int d = depth; // input disk index under inspection

	// spaces reachable by placing leaves next to the branch head
	Fundament leafReach;
	if (d < input.size() && 2 == input[d].depth) {
		leafReach = base.reachable(head, 1);

		// advance to next non-leaf
		while (d < input.size() && 2 <= input[d].depth)
			d++;
	}
	else {
		leafReach.mask.set(); // nothing reachable by leaves on branch head
	}
	d--; // special case: make sure not to skip input[d] later

	// spaces reachable by placing spines and their descendants
	Fundament extReach;
	extReach.mask.set();

	// candidate spaces for the next spine
	Fundament spinePlaces;
	spinePlaces.mask = 0x1ffeffful; // unblocked center = spine head

	while (d < input.size() && !spinePlaces.mask.all()) {
		// determine reach = max depth of nodes on current spine
		int reach = 0;
		d++; // advance from previous spine (or special case see above)
		while (d < input.size() && 0 != input[d].depth) {
			if (input[d].depth > reach)
				reach = input[d].depth;

			d++;
		}

		// unblock all within reach from all candidate spine locations
		for (int bit = 0; bit < 25; bit++) {
			if (!spinePlaces.mask.test(bit)) {
				extReach.mask &= base.reachable(Fundament::at(bit), reach).mask;
			}
		}

		// determine all successor candidate spine locations
		Fundament nextSpinePlaces;
		nextSpinePlaces.mask.set(); // block all

		for (int bit = 0; bit < 25; bit++) {
			if (!spinePlaces.mask.test(bit)) {
				nextSpinePlaces.mask &= base.reachableBySpine(Fundament::at(bit)).mask;
			}
		}

		extReach.mask &= nextSpinePlaces.mask; // unblock locations reachable by spine alone
		spinePlaces = nextSpinePlaces;
	}

	Fundament result;
	result.mask = leafReach.mask & extReach.mask;
	return result;
}


namespace
{
	/**
	 * Return @c true if problem @c rhs should be expanded before problem @c lhs.
	 */
	bool priority(const DynamicProblem& lhs, const DynamicProblem& rhs)
	{
		return lhs.depth() < rhs.depth();
	}
}

ProblemQueue::ProblemQueue()
	: open_(&priority), closed_(1024, &hash) // arbitrary # of buckets
{
}

const DynamicProblem& ProblemQueue::top() const noexcept
{
	return open_.top();
}

void ProblemQueue::push(const DynamicProblem& problem)
{
	auto signature = problem.signature();

	if (closed_.contains(signature))
		return;

	open_.push(problem);
	closed_.insert(signature);
}

void ProblemQueue::pop()
{
	open_.pop();
}

bool ProblemQueue::empty() const noexcept
{
	return open_.empty();
}

std::size_t ProblemQueue::hash(const Signature& signature) noexcept
{
	unsigned long long funValue = signature.fundament.mask.to_ullong();
	unsigned long long depth = signature.depth;
	unsigned long long headX = signature.head.x;
	unsigned long long headSly = signature.head.sly;

	// hash layout assuming 64-bit size_t:
	// 64     56      48      40      32      24      16       8       0
	// |-------|-------|-------|-------|-------|-------|-------|-------|
	// |   depth    |   headX    |  headSly   |        funValue        |
	return (depth << 51) + (headX << 38) + (headSly << 25) + funValue;
}

bool ProblemQueue::equivalent(const DynamicProblem& lhs, const DynamicProblem& rhs) noexcept
{
	return lhs.signature() == rhs.signature();
}

#include "output/svg.h"

bool DynamicProblemEmbedder::embed(std::vector<Disk>& disks)
{
	// performance counters
	int pushCounter = 0;
	int popCounter = 0;

	ProblemQueue queue;
	queue.push(DynamicProblem(disks));
	pushCounter++;

	std::ofstream stream(format("fun/contours.html"));
	Svg svg(stream);
	svg.intro();

	while (!queue.empty()) {
		const DynamicProblem& next = queue.top();

		if (next.depth() == disks.size()) {
			// accept solution - this solves all parent problems
			next.solution().apply();
			break;
		}

		auto subproblems = next.subproblems();
		queue.pop();
		popCounter++;

		for (const DynamicProblem& problem : subproblems) {
			queue.push(problem);
			pushCounter++;

			if (disks.size() - problem.depth() <= 2) {
				std::string label = format("Depth {} Hash {}", problem.depth(), ProblemQueue::hash(problem.signature()));
				//svg.write(problem.signature(), label);
			}
		}
	}

	svg.outro();
	stream.close();

	trace("Dynamic Problems: {} generated, {} expanded.", pushCounter, popCounter);

	if (queue.empty()) {
		// no embedding found - mark all disks failed
		for (Disk& disk : disks) {
			disk.failure = true;
		}
		trace("No solution found.");
	}

	return !queue.empty();
}
