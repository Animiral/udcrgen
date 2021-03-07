#include "graph.h"
#include <cassert>

void Caterpillar::extend(int leaves)
{
	leaves_.push_back(leaves);
}

const std::vector<int>& Caterpillar::leaves() const noexcept
{
	return leaves_;
}

Caterpillar Caterpillar::fromText(std::istream& stream)
{
	Caterpillar caterpillar;
	int token;

	while (stream >> token) {
		caterpillar.extend(token);
	}

	if (stream.bad()) {
		throw std::exception("Failed to parse caterpillar.");
	}

	return caterpillar;
}

UdcrGraph::UdcrGraph(int vertices, int spine)
	: vertices_(vertices), spine_(spine)
{
	assert(spine > 0);
	assert(vertices >= spine);
}

int UdcrGraph::spine() const noexcept
{
	return spine_;
}

std::vector<UdcrVertex>& UdcrGraph::vertices() noexcept
{
	return vertices_;
}

const std::vector<UdcrVertex>& UdcrGraph::vertices() const noexcept
{
	return vertices_;
}
