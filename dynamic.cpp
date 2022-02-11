#include "dynamic.h"
#include <algorithm>

Fundament::Fundament() noexcept = default;

Fundament::Fundament(const Fundament& rhs) noexcept = default;

Fundament::Fundament(const Grid& grid, Coord spineHead) noexcept
{
	for (int x = -2; x <= 2; x++) {
		for (int sly = -x-2; sly <= 2-x; sly++) {
			Coord c{ spineHead.x + x, spineHead.sly + sly };
			bool blocked = grid.at(c) != nullptr;
			int n = (sly + x + 2) * 5 + (x + 2);
			mask.set(n, blocked);
		}
	}
}

bool Fundament::operator==(const Fundament& rhs) const noexcept = default;

bool Fundament::blocked(Coord c) const noexcept
{
	int n = (c.sly + c.x + 2) * 5 + (c.x + 2);
	return mask.test(n);
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
	Rel rels[6] = { Rel::FWD_UP, Rel::FORWARD, Rel::FWD_DOWN, Rel::BACK, Rel::BACK_UP, Rel::BACK_DOWN };
	Coord candidates[6];
	std::transform(rels, rels + 6, candidates, [this, head](Rel rel)
		{ return solution_.step(head, Dir::RIGHT, rel); });

	Coord* begin = candidates;
	Coord* end = candidates + 6;

	// spine is x-monotone -> limit to forward placement options
	if (0 == disk->depth) {
		end = candidates + 3;
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
	Signature signature;
	signature.depth = static_cast<int>(solution_.size());
	signature.fundament = Fundament(solution_, spineHead_);
	signature.head = { branchHead_.x - spineHead_.x, branchHead_.sly - spineHead_.sly };

	// derive transformed alternative from fundament by mirroring, choose one.
	std::bitset<25> mirrored = signature.fundament.mask;

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
	if (mirrored.to_ulong() < signature.fundament.mask.to_ulong()) {
		signature.fundament.mask = mirrored;
		signature.head.x += signature.head.sly;
		signature.head.sly = -signature.head.sly;
	}

	return signature;
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

void ProblemQueue::push(DynamicProblem problem)
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
