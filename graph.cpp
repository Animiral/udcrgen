#include "graph.h"
#include <algorithm>
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

EdgeList edgesFromText(std::istream& stream)
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

EdgeList::iterator separateLeaves(EdgeList::iterator begin, EdgeList::iterator end)
	const std::size_t es = end - begin; // nr of edges
	const std::size_t vs = es * 2; // nr of vertice ids (non-unique)

	std::vector<int> vertices(vs);

	for (std::size_t i = 0; i < es; i++) {
		vertices[i * 2] = begin[i].from;
		vertices[i * 2 + 1] = begin[i].to;
	}

	sort(vertices.begin(), vertices.end());

	auto newEnd = end; // track removed edges

	const auto n = vertices.size();
	for (std::size_t i = 0; i < n; ) {
		if (i + 1 < n && vertices[i] == vertices[i + 1]) {
			// vertex id not unique - skip all occurrences
			do i++; while (vertices[i - 1] == vertices[i] && i + 1 < n);
		}
		else {
			// unique - remove edge
			EdgeList::iterator leafEdge;

			for (auto it = begin; it != newEnd; ++it) {
				if (it->to == vertices[i]) {
					leafEdge = it;
					break;
				}

				if (it->from == vertices[i]) {
					std::swap(it->from, it->to); // point edge to the leaf
					leafEdge = it;
					break;
				}
			}

			std::swap(*leafEdge, *--newEnd);
			i++;
		}
	}

	return newEnd;
}

/**
 * Determine whether the given edge list describes a path - a series of vertices
 * connected in a row. Only a path can be the spine of a caterpillar or lobster.
 *
 * If the edges do indeed describe a path, re-order them in the order in which
 * they can be traversed from beginning to end.
 *
 * @return true if the edges describe a path, false otherwise.
 */
bool recognizePath(EdgeList::iterator begin, EdgeList::iterator end)
{
	if (begin == end)
		return false; // sanity check - the empty graph is not a path

	// TODO! This implementation does not yet check that all vertices on the path are unique.

	const std::size_t es = end - begin; // nr of edges

	// [front, back] is the range of edges not yet on the path
	auto front = begin;
	auto back = end;

	// follow the edges along the path and swap them into forward order
	int last = begin->to; // vertex id at end of path
	int next = last;

	do {
		last = next;
		for (auto it = front; it != back; ++it) {
			if (it->from == last) {
				next = it->to;
				std::swap(*front++, *it);
				break;
			}

			if (it->to == last) {
				next = it->from;
				std::swap(it->from, it->to); // reverse edge
				std::swap(*front++, *it);
				break;
			}
		}
	} while (next != last);

	// follow the edges back to the start of the path, swap them in reverse into the back
	int start = begin->from; // vertex id at start of path
	next = start;

	do {
		start = next;
		for (auto it = front; it != back; ++it) {
			if (it->to == start) {
				next = it->from;
				std::swap(*--back, *it);
				break;
			}

			if (it->from == start) {
				next = it->to;
				std::swap(it->from, it->to); // reverse edge
				std::swap(*--back, *it);
				break;
			}
		}
	} while (next != start);

	return front == back; // is path if we have visited all the edges
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
		v.failure = false;
	}

	int spineId = 0; // attach the number of leaves to this spine vertex

	for (int leaves : caterpillar.leaves()) {
		for (int leaf = 0; leaf < leaves; leaf++) {
			auto& v = result.disks()[id + leaf];
			v.id = id + leaf;
			v.parent = spineId;
			v.failure = false;
		}

		id += leaves;
		spineId++;
	}

	return result;
}
