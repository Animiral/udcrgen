// Unit tests for graph functions

#include "gtest/gtest.h"
#include "utility/graph.h"

/**
 * Ensure that we can convert a text representation to a Caterpillar graph.
 */
TEST(Graph, Caterpillar_fromText)
{
	const std::string input = "3 4 1\n";
	std::istringstream stream{ input };
	const auto result = Caterpillar::fromText(stream);
	EXPECT_EQ(3, result.leaves().size()) << "expected 3 spine vertices, but actually " << result.leaves().size();
	EXPECT_EQ(2, result.leaves().at(0)) << "expected 2 leaves at [0], but actually " << result.leaves().at(0);
	EXPECT_EQ(2, result.leaves().at(1)) << "expected 2 leaves at [1], but actually " << result.leaves().at(1);
	EXPECT_EQ(0, result.leaves().at(2)) << "expected no leaf at [2], but actually " << result.leaves().at(2);
}

/**
 * Ensure that we can convert a text representation to a vector of Edges.
 */
TEST(Graph, edges_from_text)
{
	const std::string input =
		"5 3\n"
		"6 3\n"
		"9 3\n"
		"4 3\n"
		"7 4\n"
		"8 4\n"
		"11 8\n";
	std::istringstream stream{ input };
	const auto result = edges_from_text(stream);
	EXPECT_EQ(7, result.size()) << "expected 7 edges, but actually " << result.size();
	EXPECT_EQ(5, result.at(0).from) << "expected Edge 0 from 5, but actually from " << result.at(0).from;
	EXPECT_EQ(6, result.at(1).from) << "expected Edge 1 from 6, but actually from " << result.at(1).from;
	EXPECT_EQ(3, result.at(1).to) << "expected Edge 1 to 3, but actually to " << result.at(1).to;
}

bool edges_equal(Edge e, Edge f)
{
	return e.from == f.from && e.to == f.to;
}

TEST(Graph, separate_leaves)
{
	EdgeList graph{ {3, 5}, {4, 3}, {7, 4} };

	const EdgeList expected{ {4, 3}, {4, 7}, {3, 5} };
	const auto result = separate_leaves(graph.begin(), graph.end());
	EXPECT_EQ(result, graph.begin() + 1) << "expected: One non-leaf edge";
	EXPECT_TRUE(equal(graph.begin(), graph.end(), expected.begin(), edges_equal)) << "expected: Leaf edges are removed to the back, pointing outwards";
}

TEST(Graph, recognize_path)
{
	EdgeList graph{ {3, 5}, {4, 3}, {7, 4} };

	EXPECT_TRUE(recognize_path(graph.begin(), graph.end())) << "expected: The input graph is a path";
	const EdgeList expected{ {5, 3}, {3, 4}, {4, 7} };
	EXPECT_TRUE(equal(graph.begin(), graph.end(), expected.begin(), edges_equal)) << "expected: Path edges become aligned";
}

TEST(Graph, recognize_path_continuous)
{
	EdgeList graph{ {3, 2}, {1, 2}, {3, 4} }; // from edge 0, one leads forward, the other backward

	const EdgeList expected{ {1, 2}, {2, 3},  {3, 4} };
	EXPECT_TRUE(recognize_path(graph.begin(), graph.end())) << "expected: The input graph is a path";
	EXPECT_TRUE(equal(graph.begin(), graph.end(), expected.begin(), edges_equal)) << "expected: The input path is in continuous order";
}

/**
 * Ensure that we can convert a Lobster to a graph.
 */
TEST(Graph, DiskGraph_fromLobster)
{
	const auto NB = Lobster::NO_BRANCH;
	Lobster lobster({ {2, 1, NB, NB, NB}, {NB, NB, NB, NB, NB} });

	auto graph = DiskGraph::fromLobster(lobster);
	auto spines = graph.spines();
	auto branches = graph.branches();
	auto leaves = graph.leaves();

	EXPECT_EQ(graph.size(), 7);
	
	ASSERT_EQ(spines.size(), 2);
	EXPECT_EQ(spines[0].children, 2);
	EXPECT_EQ(spines[1].children, 0);

	ASSERT_EQ(branches.size(), 2);
	EXPECT_EQ(branches[0].parent, spines[0].id);
	EXPECT_EQ(branches[1].parent, spines[0].id);
	EXPECT_EQ(branches[0].children, 2);
	EXPECT_EQ(branches[1].children, 1);

	ASSERT_EQ(leaves.size(), 3);
	EXPECT_EQ(leaves[0].parent, branches[0].id);
	EXPECT_EQ(leaves[1].parent, branches[0].id);
	EXPECT_EQ(leaves[2].parent, branches[1].id);
}
