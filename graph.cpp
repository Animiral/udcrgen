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

std::vector<Edge> edgesFromText(std::istream& stream)
{
	std::vector<Edge> edges;

	// read all edges from input stream
	int from, to;

	while (stream >> from >> to) {
		Edge e{ from, to };
		edges.push_back(e);
	}

	if (stream.bad()) {
		throw std::exception("Failed to parse edge list.");
	}

	return edges;
}

DiskGraph::DiskGraph(int disks, int spine)
	: disks_(disks), spine_(spine)
{
	assert(spine >= 0);
	assert(disks >= spine);
}

int DiskGraph::spine() const noexcept
{
	return spine_;
}

std::vector<Disk>& DiskGraph::disks() noexcept
{
	return disks_;
}

const std::vector<Disk>& DiskGraph::disks() const noexcept
{
	return disks_;
}

const Disk& DiskGraph::findDisk(int id) const
{
	const auto it = std::find_if(disks_.begin(), disks_.end(),
		[id](const Disk& v) { return v.id == id; });

	if (it == disks_.end()) {
		throw std::exception("Vertex does not exist.");
	}

	return *it;
}

DiskGraph DiskGraph::fromCaterpillar(const Caterpillar& caterpillar)
{
	DiskGraph result{ caterpillar.countVertices(), caterpillar.countSpine() };

	int id = 0;
	for (; id < result.spine(); id++) {
		auto& v = result.disks()[id];
		v.id = id;
		v.parent = id - 1;
		v.rank = 0;
		v.failure = false;
	}

	int spineId = 0; // attach the number of leaves to this spine vertex

	for (int leaves : caterpillar.leaves()) {
		for (int leaf = 0; leaf < leaves; leaf++) {
			auto& v = result.disks()[id + leaf];
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
