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
		if (d < 2)
			throw std::exception("Caterpillar spine cannot have less than 2 leaves.");

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

	while (stream >> from >> to) {
		Edge e{ from, to };
		edges.push_back(e);
	}

	if (stream.bad()) {
		throw std::exception("Failed to parse edge list.");
	}

	return edges;
}

void edges_to_text(std::ostream& stream, const EdgeList& edges)
{
	for (const auto& edge : edges) {
		stream << edge.from << " " << edge.to << "\n";
	}

	if (stream.bad()) {
		throw std::exception("Failed to write edge list.");
	}
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

DiskGraph::DiskGraph()
	: spines_(1), branches_(), leaves_() // 1 spine for decoration
{
}

DiskGraph::DiskGraph(int spines, int branches, int leaves)
	: spines_(spines), branches_(branches), leaves_(leaves)
{
}

std::vector<Disk>& DiskGraph::spines() noexcept
{
	return spines_;
}

const std::vector<Disk>& DiskGraph::spines() const noexcept
{
	return spines_;
}

std::vector<Disk>& DiskGraph::branches() noexcept
{
	return branches_;
}

const std::vector<Disk>& DiskGraph::branches() const noexcept
{
	return branches_;
}

std::vector<Disk>& DiskGraph::leaves() noexcept
{
	return leaves_;
}

const std::vector<Disk>& DiskGraph::leaves() const noexcept
{
	return leaves_;
}

std::size_t DiskGraph::size()  const noexcept
{
	return spines_.size() + branches_.size() + leaves_.size();
}

Disk* DiskGraph::findDisk(DiskId id)
{
	const auto hasId = [id](const Disk& v) { return v.id == id; };

	auto it = std::find_if(spines_.begin(), spines_.end(), hasId);

	if (it != spines_.end())
		return &*it;

	it = std::find_if(branches_.begin(), branches_.end(), hasId);

	if (it != branches_.end())
		return &*it;

	it = std::find_if(leaves_.begin(), leaves_.end(), hasId);

	if (it != leaves_.end())
		return &*it;

	return nullptr;
}

const Disk* DiskGraph::findDisk(DiskId id) const
{
	return const_cast<DiskGraph*>(this)->findDisk(id); // reuse implementation
}

DiskGraph DiskGraph::fromCaterpillar(const Caterpillar& caterpillar)
{
	int spinesCount = caterpillar.countSpine();
	int leavesCount = caterpillar.countVertices() - spinesCount;
	DiskGraph result{ spinesCount, leavesCount, 0 };

	int id = 0;
	for (; id < spinesCount; id++) {
		auto& v = result.spines()[id];
		v.id = id;
		v.parent = id - 1;
		v.depth = 0;
		v.failure = false;
	}

	int spineId = 0; // attach the number of leaves to this spine vertex

	id = 0;
	for (int leaves : caterpillar.leaves()) {
		for (int leaf = 0; leaf < leaves; leaf++) {
			auto& v = result.branches()[id + leaf];
			v.id = spinesCount + id + leaf;
			v.parent = spineId;
			v.depth = 1;
			v.failure = false;
		}

		id += leaves;
		spineId++;
	}

	return result;
}

DiskGraph DiskGraph::fromLobster(const Lobster& lobster)
{
	const auto& spine = lobster.spine();

	int spinesCount = spine.size();
	int branchCount = 0;
	int leavesCount = 0;

	for (int i = 0; i < spinesCount; i++)
		for (int j = 0; j < 5; j++)
			if (spine[i][j] != Lobster::NO_BRANCH) {
				branchCount++;
				leavesCount += spine[i][j];
			}

	DiskGraph result{ spinesCount, branchCount, leavesCount };

	int id = 0;
	int branchIx = 0;
	int leafIx = 0;

	for (int i = 0; i < spinesCount; i++) {
		auto& s = result.spines()[i];
		s.id = id++;
		s.parent = 0 == i ? -1 : result.spines()[i - 1].id;
		s.depth = 0;
		s.children = 0;
		s.embedded = false;

		for (int j = 0; j < 5; j++) {
			if (Lobster::NO_BRANCH == spine[i][j])
				continue;

			s.children++;
			auto& b = result.branches()[branchIx++];
			b.id = id++;
			b.parent = s.id;
			b.depth = 1;
			b.children = spine[i][j];
			b.embedded = false;

			for (int k = 0; k < spine[i][j]; k++) {
				auto& l = result.leaves()[leafIx++];
				l.id = id++;
				l.parent = b.id;
				l.depth = 2;
				l.children = 0;
				l.embedded = false;
			}
		}
	}

	return result;
}

EdgeList DiskGraph::toEdgeList() const
{
	EdgeList edgeList;

	for (const auto& spine : spines()) {
		if (spine.parent > 0)
			edgeList.push_back({ spine.parent, spine.id });
	}

	for (const auto& branch : branches()) {
		edgeList.push_back({ branch.parent, branch.id });
	}

	for (const auto& leaf : leaves()) {
		edgeList.push_back({ leaf.parent, leaf.id });
	}

	return edgeList;
}
