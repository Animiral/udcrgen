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


GraphTraversal::GraphTraversal() noexcept
	: disk_(nullptr), order_()
{
}

GraphTraversal::GraphTraversal(Disk* from, Configuration::EmbedOrder order) noexcept
	: disk_(from), order_(order)
{
}

GraphTraversal& GraphTraversal::operator++() noexcept
{
	assert(disk_);

	switch (order_) {
	case Configuration::EmbedOrder::DEPTH_FIRST:
		if (disk_->child) {
			disk_ = disk_->child;
			return *this;
		}

		while (!disk_->nextSibling && disk_->parent)
			disk_ = disk_->parent;

		break;

	case Configuration::EmbedOrder::BREADTH_FIRST:
		if (0 == disk_->depth && disk_->child) {
			disk_ = disk_->child;
			return *this;
		}

		if (0 < disk_->depth && !disk_->nextSibling) {
			if (1 == disk_->depth) {
				disk_ = disk_->parent->child;
			}
			else {
				assert(2 == disk_->depth);

				if (disk_->parent->nextSibling)
					disk_ = disk_->parent->nextSibling;
				else {
					disk_ = disk_->parent->parent;
					goto next_spine;
				}
			}

			// search sibling branches for leaves
			while (!disk_->child && disk_->nextSibling)
				disk_ = disk_->nextSibling;

			if (disk_->child) {
				disk_ = disk_->child;
				return *this;
			}
			else {
				disk_ = disk_->parent; // next spine
			}
		}

		break;

	default:
		assert(0);

	}

next_spine:
	disk_ = disk_->nextSibling;
	return *this;
}

Disk& GraphTraversal::operator*() const noexcept
{
	return *disk_;
}

Disk* GraphTraversal::operator->() const noexcept
{
	return disk_;
}

bool GraphTraversal::operator==(const GraphTraversal& rhs) const noexcept
{
	return disk_ == rhs.disk_;
}

bool GraphTraversal::operator!=(const GraphTraversal& rhs) const noexcept
{
	return disk_ != rhs.disk_;
}


DiskGraph::DiskGraph(std::vector<Disk>&& disks, Disk* tip)
	: disks_(move(disks)), tip_(tip)
{
	if (disks_.empty()) {
		assert(!tip_);
		return; // leave tip unchanged
	}

	if (!tip_)
		tip_ = &disks_[0];

	assert(!tip_->prevSibling);
	assert(!tip_->parent);
}

DiskGraph::DiskGraph(const DiskGraph& rhs)
	: disks_(rhs.disks_), tip_(rhs.tip_)
{
	fixDiskPointer(rhs, tip_);

	for (Disk& disk : disks_) {
		fixDiskPointer(rhs, disk.parent);
		fixDiskPointer(rhs, disk.prevSibling);
		fixDiskPointer(rhs, disk.nextSibling);
		fixDiskPointer(rhs, disk.child);
	}
}

DiskGraph::DiskGraph(DiskGraph&& rhs) noexcept = default;

DiskGraph& DiskGraph::operator=(const DiskGraph& rhs)
{
	disks_ = rhs.disks_;
	tip_ = rhs.tip_;

	fixDiskPointer(rhs, tip_);

	for (Disk& disk : disks_) {
		fixDiskPointer(rhs, disk.parent);
		fixDiskPointer(rhs, disk.prevSibling);
		fixDiskPointer(rhs, disk.nextSibling);
		fixDiskPointer(rhs, disk.child);
	}

	return *this;
}

DiskGraph& DiskGraph::operator=(DiskGraph&& rhs) noexcept = default;

std::vector<Disk>& DiskGraph::disks() noexcept
{
	return disks_;
}

Disk* DiskGraph::tip() noexcept
{
	return tip_;
}

const std::vector<Disk>& DiskGraph::disks() const noexcept
{
	return disks_;
}

const Disk* DiskGraph::tip() const noexcept
{
	return tip_;
}

GraphTraversal DiskGraph::traversal(Configuration::EmbedOrder order) noexcept
{
	return GraphTraversal(tip_, order);
}

GraphTraversal DiskGraph::end() const noexcept
{
	return {};
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

DiskGraph DiskGraph::fromCaterpillar(const Caterpillar& caterpillar)
{
	std::vector<Disk> disks(caterpillar.countVertices());
	Disk* tip = nullptr;

	DiskId id = 0;

	for (int leaves : caterpillar.leaves()) {
		auto& spine = disks[id];
		spine.id = id;
		spine.parent = nullptr;
		spine.prevSibling = nullptr;
		spine.nextSibling = tip;
		tip->prevSibling = &spine;
		tip = &spine;
		spine.child = nullptr;
		spine.depth = 0;
		spine.failure = false;
		id++;

		for (int l = 0; l < leaves; l++) {
			auto& leaf = disks[id];
			leaf.id = id;
			leaf.parent = &spine;
			leaf.prevSibling = nullptr;
			leaf.nextSibling = spine.child;
			spine.child->prevSibling = &leaf;
			spine.child = &leaf;
			leaf.child = nullptr;
			leaf.depth = 1;
			leaf.failure = false;
			id++;
		}
	}

	return DiskGraph{ move(disks), tip };
}

DiskGraph DiskGraph::fromLobster(const Lobster& lobster)
{
	std::vector<Disk> disks(lobster.countVertices());
	Disk* tip = nullptr;

	const auto& lobst = lobster.spine();
	DiskId id = lobster.countVertices() - 1;

	// create spines and branches in "reverse" because
	// of the way sibling/child pointers are linked up
	for (int i = lobst.size() - 1; i >= 0; i--) {
		std::size_t descendents = std::reduce(lobst[i].begin(), lobst[i].end(), 5);
		auto& spine = disks[id - descendents];
		spine.id = id - descendents;
		spine.parent = nullptr;
		spine.prevSibling = nullptr;
		spine.nextSibling = tip;
		if (tip)
			tip->prevSibling = &spine;
		tip = &spine;
		spine.child = nullptr;
		spine.depth = 0;
		spine.children = 0;
		spine.embedded = false;

		for (int j = 4; j >= 0; j--) {
			int leaves = lobst[i][j];

			if (Lobster::NO_BRANCH == leaves)
				continue;

			spine.children++;
			auto& branch = disks[id - leaves];
			branch.id = id - leaves;
			branch.parent = &spine;
			branch.prevSibling = nullptr;
			branch.nextSibling = spine.child;
			if (spine.child)
				spine.child->prevSibling = &branch;
			spine.child = &branch;
			branch.child = nullptr;
			branch.depth = 1;
			branch.children = leaves;
			branch.embedded = false;

			for (int k = 0; k < lobst[i][j]; k++) {
				auto& leaf = disks[id - k];
				leaf.id = id - k;
				leaf.parent = &branch;
				leaf.prevSibling = nullptr;
				leaf.nextSibling = branch.child;
				if (branch.child)
					branch.child->prevSibling = &leaf;
				branch.child = &leaf;
				leaf.child = nullptr;
				leaf.depth = 2;
				leaf.children = 0;
				leaf.embedded = false;
			}

			id -= leaves + 1;
		}

		id--; // skip over the spine's own id
	}

	return DiskGraph{ move(disks), tip };
}

EdgeList DiskGraph::toEdgeList() const
{
	EdgeList edgeList;

	for (const Disk& disk : disks())
		if (disk.parent)
			edgeList.push_back({ disk.parent->id, disk.id });

	return edgeList;
}

// Ensure that the pointer which was copied from the base graph
// points to the equivalent object in the copy graph.
void DiskGraph::fixDiskPointer(const DiskGraph& base, Disk*& pointer) noexcept
{
	if (pointer)
		pointer = pointer
			- const_cast<Disk*>(&*base.disks_.begin())
			+ &*disks_.begin();
}
