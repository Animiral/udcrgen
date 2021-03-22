#include "graph.h"
#include <cassert>
#include <numeric>

void Caterpillar::extend(int leaves)
{
	assert(leaves >= 0);
	leaves_.push_back(leaves);
}

int Caterpillar::countVertices() const noexcept
{
	return countSpine() + std::accumulate(leaves_.begin(), leaves_.end(), 0);
}

int Caterpillar::countSpine() const noexcept
{
	return static_cast<int>(leaves_.size());
}

const std::vector<int>& Caterpillar::leaves() const noexcept
{
	return leaves_;
}

Caterpillar Caterpillar::fromText(std::istream& stream)
{
	std::vector<int> degrees; // vertex degrees from degree representation
	int token;

	while (stream >> token) {
		degrees.push_back(token);
	}

	if (stream.bad()) {
		throw std::exception("Failed to parse caterpillar.");
	}

	// correction: first and last spine have leaves = degree - 1,
	// mid-spine have leaves = degree - 2
	if (degrees.size() > 0)
	{
		++degrees.front();
		++degrees.back();
	}

	Caterpillar caterpillar;

	for (int d : degrees) {
		caterpillar.extend(d - 2);
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

const UdcrVertex& UdcrGraph::findVertex(int id) const
{
	const auto it = std::find_if(vertices_.begin(), vertices_.end(),
		[id](const UdcrVertex& v) { return v.id == id; });

	if (it == vertices_.end()) {
		throw std::exception("Vertex does not exist.");
	}

	return *it;
}

UdcrGraph UdcrGraph::fromCaterpillar(const Caterpillar& caterpillar)
{
	UdcrGraph result{ caterpillar.countVertices(), caterpillar.countSpine() };

	int id = 0;
	for (; id < result.spine(); id++) {
		auto& v = result.vertices()[id];
		v.id = id;
		v.parent = id - 1;
		v.rank = 0;
		v.failure = false;
	}

	int spineId = 0; // attach the number of leaves to this spine vertex

	for (int leaves : caterpillar.leaves()) {
		for (int leaf = 0; leaf < leaves; leaf++) {
			auto& v = result.vertices()[id + leaf];
			v.id = id + leaf;
			v.parent = spineId;
			v.rank = leaf;
			v.failure = false;
		}

		id += leaves;
		spineId++;
	}

	return result;
}
