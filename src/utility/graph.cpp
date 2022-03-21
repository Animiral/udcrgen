#include "graph.h"
#include "input/input.h" // TODO: remove dependent functions from utility/ structure
#include "exception.h"
#include <algorithm>
#include <unordered_map>
#include <string>
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

	while (readint(stream, token)) {
		degrees.push_back(token);
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
		if (d < 2)
			throw InputException("Caterpillar spine cannot have degree <2.");

		caterpillar.extend(d - 2);
	}

	return caterpillar;
}

Lobster::Lobster() noexcept = default;

Lobster::Lobster(Lobster&& ) noexcept = default;

Lobster::Lobster(std::vector<Spine> spine) noexcept
	: spine_(move(spine))
{
}

Lobster& Lobster::operator=(Lobster&& ) noexcept = default;

bool Lobster::operator==(const Lobster& rhs) const noexcept
{
	return spine_ == rhs.spine_;
}

int Lobster::countVertices() const noexcept
{
	int count = countSpine();

	for (int i = 0; i < spine_.size(); i++) {
		count += std::accumulate(spine_[i].begin(), spine_[i].end(), 5);
	}

	return count;
}

int Lobster::countSpine() const noexcept
{
	return spine_.size();
}

const std::vector<Lobster::Spine>& Lobster::spine() const noexcept
{
	return spine_;
}

std::vector<Lobster::Spine>& Lobster::spine() noexcept
{
	return spine_;
}

EdgeList edges_from_text(std::istream& stream)
{
	std::vector<Edge> edges;

	// read all edges from input stream
	DiskId from, to;

	while (readint(stream, from) && readint(stream, to)) { // TODO: only one int of an edge is an error!
		Edge e{ from, to };
		edges.push_back(e);
		ignoreline(stream);
	}

	return edges;
}

void edges_to_text(std::ostream& stream, const EdgeList& edges)
{
	for (const auto& edge : edges) {
		stream << edge.from << " " << edge.to << "\n";
	}

	if (stream.fail())
		throw OutputException(std::strerror(errno));
}

EdgeList::iterator separate_leaves(EdgeList::iterator begin, EdgeList::iterator end)
{
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

bool recognize_path(EdgeList::iterator begin, EdgeList::iterator end)
{
	if (begin == end)
		return true; // the empty graph is also a path

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

	// remember where the forward path ends
	auto const cut = front;

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

	if (front != back)
		return false; // we have not been able to cover all the edges

	std::rotate(begin, cut, end); // make the path continuous
	return true;
}

DiskGraph::DiskGraph(std::vector<Disk>&& disks)
	: disks_(move(disks))
{
}

std::vector<Disk>& DiskGraph::disks() noexcept
{
	return disks_;
}

const std::vector<Disk>& DiskGraph::disks() const noexcept
{
	return disks_;
}

std::size_t DiskGraph::size()  const noexcept
{
	return disks_.size();
}

std::size_t DiskGraph::length() const noexcept
{
	return std::ranges::count_if(disks_, [](const Disk & d) { return 0 == d.depth; });
}

Disk* DiskGraph::findDisk(DiskId id)
{
	auto it = std::ranges::find(disks_, id, [](const Disk& d) { return d.id; });

	if (it != disks_.end())
		return &*it;

	return nullptr;
}

const Disk* DiskGraph::findDisk(DiskId id) const
{
	return const_cast<DiskGraph*>(this)->findDisk(id); // reuse implementation
}

void DiskGraph::reorder(Configuration::EmbedOrder order)
{
	// Primary order: spine position.
	std::unordered_map<DiskId, std::size_t> primary;

	const std::size_t length = this->length();

	// TODO: more efficient implementation (when disk structure exists)
	// assign primary order to spine nodes
	DiskId parent = NODISK;
	for (std::size_t p = 0; p < length; p++) {
		auto it = std::ranges::find_if(disks_, [parent](const Disk& d) { return 0 == d.depth && parent == d.parent; });
		assert(disks_.end() != it);
		primary[it->id] = p;
		parent = it->id;
	}

	// propragate primary order to branches and spines
	while (primary.size() < disks_.size()) {
		// TODO: inefficiency
		// find a disk that needs a primary entry
		const Disk* disk = nullptr;
		for (std::size_t j = 0; j < disks_.size(); j++) {
			disk = &disks_[j];
			if (!primary.contains(disk->id)
				&& primary.contains(disk->parent)) {
				break;
			}
		}
		assert(disk);
		primary[disk->id] = primary[disk->parent];
	}

	auto before = [&primary, order](const Disk& a, const Disk& b)
	{
		auto pa = primary[a.id], pb = primary[b.id];

		if (pa == pb) { // same spine
			if (a.depth > 0 && b.depth > 0
				&& Configuration::EmbedOrder::DEPTH_FIRST == order) {
				int ba = 1 == a.depth ? a.id : a.parent;
				int bb = 1 == b.depth ? b.id : b.parent;

				if (ba == bb) { // same branch
					return a.depth < b.depth;
				}
				else {
					return ba < bb;
				}
			}
			else {
				return a.depth < b.depth;
			}
		}
		else {
			return pa < pb;
		}
	};

	std::ranges::sort(disks_.begin(), disks_.end(), before);
}

DiskGraph DiskGraph::fromCaterpillar(const Caterpillar& caterpillar)
{
	std::vector<Disk> disks(caterpillar.countVertices());

	DiskId id = 0;
	DiskId spineId = -1;

	for (int leaves : caterpillar.leaves()) {
		auto& v = disks[id];
		v.id = id;
		v.parent = spineId;
		v.depth = 0;
		v.failure = false;
		spineId = id;
		id++;

		for (int leaf = 0; leaf < leaves; leaf++) {
			auto& v = disks[id];
			v.id = id;
			v.parent = spineId;
			v.depth = 1;
			v.failure = false;
			id++;
		}
	}

	return DiskGraph{ move(disks) };
}

DiskGraph DiskGraph::fromLobster(const Lobster& lobster)
{
	std::vector<Disk> disks(lobster.countVertices());
	const auto& spine = lobster.spine();

	DiskId id = 0;
	DiskId spineId = NODISK;

	for (int i = 0; i < spine.size(); i++) {
		auto& s = disks[id];
		s.id = id;
		s.parent = spineId;
		s.depth = 0;
		s.children = 0;
		s.embedded = false;
		spineId = id;
		id++;

		for (int j = 0; j < 5; j++) {
			if (Lobster::NO_BRANCH == spine[i][j])
				continue;

			s.children++;
			auto& b = disks[id];
			b.id = id;
			b.parent = spineId;
			b.depth = 1;
			b.children = spine[i][j];
			b.embedded = false;
			id++;

			for (int k = 0; k < spine[i][j]; k++) {
				auto& l = disks[id];
				l.id = id;
				l.parent = b.id;
				l.depth = 2;
				l.children = 0;
				l.embedded = false;
				id++;
			}
		}
	}

	return DiskGraph{ move(disks) };
}

EdgeList DiskGraph::toEdgeList() const
{
	EdgeList edgeList;

	for (const Disk& disk : disks())
		if (NODISK != disk.parent)
			edgeList.push_back({ disk.parent, disk.id });

	return edgeList;
}
