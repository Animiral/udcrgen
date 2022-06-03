#include "dynamic.h"
#include "utility/util.h"
#include "utility/log.h"
#include "utility/exception.h"
#include <algorithm>

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

void Fundament::block(Coord c) noexcept
{
	mask.set(index(c));
}

void Fundament::shift(Dir dir) noexcept
{
	// these are all our legitimate use cases
	assert(Dir::RIGHT == dir || Dir::RIGHT_UP == dir || Dir::RIGHT_DOWN == dir);

	switch (dir) {
	case Dir::RIGHT_UP:
		mask >>= 5;
		break;

	case Dir::RIGHT:
		(mask >>= 6) &= 0b01111'01111'01111'01111'01111;
		break;

	case Dir::RIGHT_DOWN:
		(mask >>= 1) &= 0b01111'01111'01111'01111'01111;
		break;

	}
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

bool Signature::dominates(const Signature& rhs) const noexcept
{
	if (depth != rhs.depth || head != rhs.head)
		return false;

	return (fundament.mask & rhs.fundament.mask) == fundament.mask;
}

DynamicProblem::DynamicProblem(DiskGraph& graph, bool constructive)
	: spineHead_({ -1, 0 }), // this causes first disk at {0, 0}
	position_(graph.traversal(Configuration::EmbedOrder::DEPTH_FIRST)),
	end_(graph.end()),
	depth_(0),
	constructive_(constructive)
{
}

DynamicProblem::DynamicProblem(std::shared_ptr<const DynamicProblem> parent, Dir dir)
	: fundament_(parent->fundament_),
	spineHead_(parent->spineHead_),
	branchHead_(parent->branchHead_),
	position_(parent->position_),
	end_(parent->end_),
	depth_(parent->depth_ + 1),
	constructive_(true),
	parent_(move(parent)) // init last, do not invalidate ptr early
{
	assert(parent_->constructive_); // otherwise we would use the other c'tor

	initPlacement(dir);
	++position_;
}

DynamicProblem::DynamicProblem(const DynamicProblem* parent, Dir dir)
	: fundament_(parent->fundament_),
	spineHead_(parent->spineHead_),
	branchHead_(parent->branchHead_),
	position_(parent->position_),
	end_(parent->end_),
	depth_(parent->depth_ + 1),
	constructive_(false)
{
	assert(!parent->constructive_); // otherwise we would use the other c'tor

	initPlacement(dir);
	++position_;
}

void DynamicProblem::initPlacement(Dir dir)
{
	switch (position_->depth) {
	case 0: // spine
		placement_ = spineHead_ + dir;
		fundament_.shift(dir);
		fundament_.block({ 0, 0 });
		spineHead_ = placement_;
		break;

	case 1: // branch
	{
		placement_ = spineHead_ + dir;
		Coord relCoord{ placement_.x - spineHead_.x, placement_.sly - spineHead_.sly };
		fundament_.block(relCoord);
		branchHead_ = placement_;
		break;
	}

	case 2: // leaf
	{
		placement_ = branchHead_ + dir;
		Coord relCoord{ placement_.x - spineHead_.x, placement_.sly - spineHead_.sly };
		fundament_.block(relCoord);
		break;
	}

	default:
		throw EmbedException("Dynamic program can not embed graphs deeper than lobsters");

	}
}

std::vector<DynamicProblem> DynamicProblem::subproblems() const
{
	int diskDepth = position_->depth;

	if (0 == depth_) { // special case
		// arbitrarily choose dir to place the first disk at (0,0)

		if (constructive_) {
			// build and include information to reconstruct the solution
			auto sharedThis = std::make_shared<const DynamicProblem>(*this);
			return { DynamicProblem(move(sharedThis), Dir::RIGHT) };
		}
		else {
			// do not maintain solution information for decision procedure
			return { DynamicProblem(this, Dir::RIGHT) };
		}
	}

	// place disk next to the appropriate head
	Coord head;
	if (2 == diskDepth) {
		head = branchHead_;
	}
	else if (diskDepth < 2) {
		head = spineHead_;
	}
	else {
		throw EmbedException("Dynamic program can not embed graphs deeper than lobsters");
	}

	// we have up to 6 choices to place the upcoming disk
	// order is significant later!
	Dir dirs[6] = { Dir::LEFT, Dir::LEFT_UP, Dir::LEFT_DOWN, Dir::RIGHT, Dir::RIGHT_UP, Dir::RIGHT_DOWN };
	Dir* begin = dirs;
	Dir* end = dirs + 6;

	// spine is x-monotone -> limit to forward placement options
	if (0 == diskDepth) {
		begin = dirs + 3;
	}

	// do not consider blocked candidate spaces
	end = std::remove_if(begin, end, [this, head](Dir d) {
		Coord c = head + d;
		return fundament_.blocked({ c.x - spineHead_.x, c.sly - spineHead_.sly });
	});

	if (begin == end)
		return {}; // skip constructing shared this

	std::vector<DynamicProblem> subproblems;

	if (constructive_) {
		// build and include information to reconstruct the solution
		auto sharedThis = std::make_shared<const DynamicProblem>(*this);

		for (auto it = begin; it != end; ++it) {
			subproblems.push_back({ sharedThis, *it });
		}
	}
	else {
		// do not maintain solution information for decision procedure
		for (auto it = begin; it != end; ++it) {
			subproblems.push_back({ this, *it });
		}
	}

	return subproblems;
}

void DynamicProblem::setState(Fundament fundament, GraphTraversal position, Coord spineHead, Coord branchHead, int depth)
{
	fundament_ = fundament;
	position_ = position;
	spineHead_ = spineHead;
	branchHead_ = branchHead;
	depth_ = depth;
}

const Fundament& DynamicProblem::fundament() const noexcept
{
	return fundament_;
}

Grid DynamicProblem::solution() const
{
	Grid solution(depth_);

	for (auto* prob = this; prob->parent_; prob = prob->parent_.get())
		solution.put(prob->placement_, *prob->parent_->position_);

	return solution;
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
	Coord head{ 0, 0 };

	// consider branch head only if it is relevant at this point (upcoming leaf)
	Disk* upcomingDisk = position_ != end_ ? &*position_ : nullptr;
	if (upcomingDisk != nullptr && 2 == upcomingDisk->depth) {
		head = { branchHead_.x - spineHead_.x, branchHead_.sly - spineHead_.sly };
	}

	// disregard unreachable spaces
	Fundament fundament = reachableEventually(fundament_, head, position_, end_);

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


Fundament reachableEventually(Fundament base, Coord head, GraphTraversal position, GraphTraversal end) noexcept
{
	// spaces reachable by placing leaves next to the branch head
	Fundament leafReach;
	if (position != end && 2 == position->depth) {
		leafReach = base.reachable(head, 1);

		// advance to next non-leaf
		while (position != end && 2 <= position->depth)
			++position;
	}
	else {
		leafReach.mask.set(); // nothing reachable by leaves on branch head
	}

	// spaces reachable by placing spines and their descendants
	Fundament extReach;
	extReach.mask.set();

	// candidate spaces for the next spine
	Fundament spinePlaces;
	spinePlaces.mask = 0x1ffeffful; // unblocked center = spine head

	while (position != end && !spinePlaces.mask.all()) {
		// determine reach = max depth of nodes on current spine
		int reach = 0;
		while (position != end && 0 != position->depth) {
			if (position->depth > reach)
				reach = position->depth;

			++position;
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

		if (position != end)
			++position; // advance from previous spine
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
		return lhs.depth() < rhs.depth(); // TODO: BFS vs DFS, consider fundament popcnt
	}
}

ProblemQueue::ProblemQueue()
	: open_(&priority), closed_(&less)
{
}

const DynamicProblem& ProblemQueue::top() const noexcept
{
	return open_.top();
}

void ProblemQueue::push(const DynamicProblem& problem)
{
	auto signature = problem.signature();

	// We want to compare the new signature to the known ones.
	// Since both open_ and closed_ are locally ordered by fundament bit count,
	// we can quickly query the few comparable entries.
	auto lowerSig = signature;
	lowerSig.fundament.mask = 0; // all unblocked
	auto upperSig = signature;
	upperSig.fundament.mask.set(); // all blocked

	// If we previously encountered a dominating signature, we have no need for the new one.
	for (auto it = closed_.lower_bound(lowerSig),
	         end = closed_.upper_bound(upperSig); it != end; ++it) {
		if (it->dominates(signature))
			return;
	}

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

bool ProblemQueue::less(const Signature& lhs, const Signature& rhs) noexcept
{
	if (lhs.depth < rhs.depth) return true;
	if (lhs.depth > rhs.depth) return false;

	if (lhs.head.x < rhs.head.x) return true;
	if (lhs.head.x > rhs.head.x) return false;

	if (lhs.head.sly < rhs.head.sly) return true;
	if (lhs.head.sly > rhs.head.sly) return false;

	if (lhs.fundament.mask.count() < rhs.fundament.mask.count()) return true;
	if (lhs.fundament.mask.count() > rhs.fundament.mask.count()) return false;

	return lhs.fundament.mask.to_ulong() < rhs.fundament.mask.to_ulong();
}

bool ProblemQueue::equivalent(const DynamicProblem& lhs, const DynamicProblem& rhs) noexcept
{
	return lhs.signature() == rhs.signature();
}

DynamicProblemEmbedder::DynamicProblemEmbedder(bool constructive) noexcept
	: WholesaleEmbedder(), constructive_(constructive)
{
}

bool DynamicProblemEmbedder::embed(DiskGraph& graph)
{
	// performance counters
	int pushCounter = 0;
	int popCounter = 0;

	ProblemQueue queue;
	queue.push(DynamicProblem(graph, constructive_));
	pushCounter++;

	while (!queue.empty()) {
		const DynamicProblem& next = queue.top();

		if (next.depth() == graph.size()) {
			// accept solution - this solves all parent problems
			if (constructive_)
				next.solution().apply();
			break;
		}

		auto subproblems = next.subproblems();
		queue.pop();
		popCounter++;

		for (const DynamicProblem& problem : subproblems) {
			queue.push(problem);
			pushCounter++;
		}
	}

	trace("Dynamic Problems: {} generated, {} expanded.", pushCounter, popCounter);

	if (queue.empty()) {
		// no embedding found - mark all disks failed
		for (Disk& disk : graph.disks()) {
			disk.failure = true;
		}
		trace("No solution found.");
	}

	return !queue.empty();
}
