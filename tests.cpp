// Various unit tests

#include "gtest/gtest.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include "graph.h"
#include "embed.h"

std::ostream& operator<<(std::ostream& stream, GraphClass gc)
{
	switch (gc) {
	case GraphClass::CATERPILLAR: return stream << "CATERPILLAR";
	case GraphClass::LOBSTER: return stream << "LOBSTER";
	case GraphClass::OTHER: return stream << "OTHER";
	default: return stream << "[?]";
	}
}

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
	EXPECT_EQ(0, result.leaves().at(2)) << "expected no leaf at [2], but actually "<< result.leaves().at(2);
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

	//const bool isPath = recognize_path(graph.begin(), graph.end());
	//EXPECT_TRUE(isPath) << "expected: The input graph is a path";
	EXPECT_TRUE(recognize_path(graph.begin(), graph.end())) << "expected: The input graph is a path";
	const EdgeList expected{ {5, 3}, {3, 4}, {4, 7} };
	//const bool matchesEdges = equal(graph.begin(), graph.end(), expected.begin(), edges_equal);
	//EXPECT_TRUE(matchesEdges) << "expected: Path edges become aligned";
	EXPECT_TRUE(equal(graph.begin(), graph.end(), expected.begin(), edges_equal)) << "expected: Path edges become aligned";
}

TEST(Graph, recognize_path_continuous)
{
	EdgeList graph{ {3, 2}, {1, 2}, {3, 4} }; // from edge 0, one leads forward, the other backward

	const EdgeList expected{ {1, 2}, {2, 3},  {3, 4} };
	EXPECT_TRUE(recognize_path(graph.begin(), graph.end())) << "expected: The input graph is a path";
	EXPECT_TRUE(equal(graph.begin(), graph.end(), expected.begin(), edges_equal)) << "expected: The input path is in continuous order";
}

TEST(Embed, classify_string)
{
	EdgeList edges{ {3, 5}, {4, 3}, {7, 4} }; // path-only caterpillar, no leaves

	const auto result = classify(edges);
	const auto& graph = result.first;

	EXPECT_EQ(GraphClass::CATERPILLAR, result.second) << "expected CATERPILLAR, but actually " << result.second;
	EXPECT_EQ(4, graph.spines().size()) << "expected 4 spine disks, but actually" << graph.spines().size();
	EXPECT_EQ(0, graph.branches().size()) << "expected no branch disks, but actually" << graph.branches().size();
	EXPECT_EQ(0, graph.leaves().size()) << "expected no leaf disks, but actually" << graph.leaves().size();
}

TEST(Embed, classify_caterpillar)
{
	EdgeList edges{ {3, 5}, {4, 3}, {7, 3} }; // caterpillar, 1 spine, 3 leaves

	const auto result = classify(edges);
	const auto& graph = result.first;

	EXPECT_EQ(GraphClass::CATERPILLAR, result.second) << "expected CATERPILLAR, but actually " << result.second;
	EXPECT_EQ(1, graph.spines().size()) << "expected 1 spine disk, but actually" << graph.spines().size();
	EXPECT_EQ(3, graph.spines()[0].id) << "expected spine disk is 3, but actually" << graph.spines()[0].id;
	EXPECT_EQ(3, graph.branches().size()) << "expected 3 branch disks, but actually" << graph.branches().size();
	EXPECT_EQ(0, graph.leaves().size()) << "expected no leaf disks, but actually" << graph.leaves().size();
}

TEST(Embed, classify_lobster)
{
	//   1 -- 5 -.
	//             3 -- 7 -- 8
	//   2 -- 4 -`
	EdgeList edges{ {3, 5}, {1, 5}, {7, 8}, {4, 3}, {4, 2}, {7, 3} }; // lobster, 1 spine, 3 branches, 3 leaves

	const auto result = classify(edges);
	const auto& graph = result.first;

	EXPECT_EQ(GraphClass::LOBSTER, result.second) << "expected LOBSTER, but actually " << result.second;
	EXPECT_EQ(1, graph.spines().size()) << "expected 1 spine disk, but actually" << graph.spines().size();
	EXPECT_EQ(3, graph.spines()[0].id) << "expected spine disk is 3, but actually" << graph.spines()[0].id;
	EXPECT_EQ(3, graph.branches().size()) << "expected 3 branch disks, but actually" << graph.branches().size();
	EXPECT_EQ(3, graph.leaves().size()) << "expected 3 leaf disks, but actually" << graph.leaves().size();
}

TEST(Embed, classify_stumped_lobster)
{
	//   1 -- 5 -.
	//            \       .- 8
	//        6 -- 3 -- 7
	//            /       `- 9
	//   2 -- 4 -`
	EdgeList edges{ {3, 6}, {3, 5}, {1, 5}, {7, 8}, {7, 9}, {4, 3}, {4, 2}, {7, 3} }; // lobster, 1 spine, 3 branches, 3 leaves

	const auto result = classify(edges);
	const auto& graph = result.first;

	EXPECT_EQ(GraphClass::LOBSTER, result.second) << "expected LOBSTER with stump, but actually " << result.second;
	EXPECT_EQ(1, graph.spines().size()) << "expected 1 spine disk, but actually" << graph.spines().size();
	EXPECT_EQ(3, graph.spines()[0].id) << "expected spine disk is 3, but actually" << graph.spines()[0].id;
	EXPECT_EQ(4, graph.branches().size()) << "expected 4 branch disks, but actually" << graph.branches().size();
	EXPECT_EQ(4, graph.leaves().size()) << "expected 4 leaf disks, but actually" << graph.leaves().size();
}
