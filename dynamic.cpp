#include "dynamic.h"
#include <algorithm>

bool Fundament::blocked(Coord c) const noexcept
{
	int n = (c.sly + c.x + 2) * 5 + (c.x + 2);
	return mask.test(n);
}

Fundament makeFundament(Grid grid, Coord head)
{
	Fundament fundament;

	for (int x = -2; x <= 2; x++) {
		for (int sly = -x-2; sly <= 2-x; sly++) {
			Coord c{ head.x + x, head.sly + sly };
			bool blocked = grid.at(c) != nullptr;
			int n = (sly + x + 2) * 5 + (x + 2);
			fundament.mask.set(n, blocked);
		}
	}

	return fundament;
}

DynamicProblem::DynamicProblem(InputDisks& input)
	: solution_(input.size()),
	spineHead_(),
	branchHead_(),
	input_(&input),
	depth_(0)
{
}

DynamicProblem::DynamicProblem(InputDisks& input, Grid solution, Coord spineHead, Coord branchHead)
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
	Fundament fundament = makeFundament(solution_, head);

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

void DynamicProblem::setSolution(Grid solution, Coord spineHead, Coord branchHead)
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
