// Various unit tests

#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include "graph.h"
#include "embed.h"

/**
 * Print a message if the result is false.
 * This will be replaced by some testing framework.
 */
void expect(bool result, const std::string& message)
{
	if (!result) {
		std::cerr << "Test failed: " << message << "\n";
	}
}

/**
 * Ensure that we can convert a text representation to a Caterpillar graph.
 */
void test_Caterpillar_fromText()
{
	const std::string input = "3 4 1\n";
	std::istringstream stream{ input };
	const auto result = Caterpillar::fromText(stream);
	expect(3 == result.leaves().size(), "Caterpillar has 3 spine vertices");
	expect(2 == result.leaves().at(0), "Caterpillar has 2 leaves at [0]");
	expect(2 == result.leaves().at(1), "Caterpillar has 2 leaves at [1]");
	expect(0 == result.leaves().at(2), "Caterpillar has no leaf at [2]");
}

/**
 * Ensure that we can convert a text representation to a vector of Edges.
 */
void test_edges_from_text()
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
	expect(7 == result.size(), "Read 7 edges");
	expect(5 == result.at(0).from, "Edge 0 from 5");
	expect(6 == result.at(1).from, "Edge 1 from 6");
	expect(3 == result.at(1).to, "Edge 1 to 3");
}

bool edges_equal(Edge e, Edge f)
{
	return e.from == f.from && e.to == f.to;
}

void test_separate_leaves()
{
	EdgeList graph{ {3, 5}, {4, 3}, {7, 4} };

	const EdgeList expected{ {4, 3}, {4, 7}, {3, 5} };
	const auto result = separate_leaves(graph.begin(), graph.end());
	expect(result == graph.begin() + 1, "One non-leaf edge");
	expect(equal(graph.begin(), graph.end(), expected.begin(), edges_equal), "Leaf edges are removed to the back, pointing outwards");
}

void test_recognize_path()
{
	EdgeList graph{ {3, 5}, {4, 3}, {7, 4} };

	const EdgeList expected{ {5, 3}, {3, 4}, {4, 7} };
	expect(recognize_path(graph.begin(), graph.end()), "The input graph is a path");
	expect(equal(graph.begin(), graph.end(), expected.begin(), edges_equal), "Path edges become aligned");
}

void test_recognize_path_continuous()
{
	EdgeList graph{ {3, 2}, {1, 2}, {3, 4} }; // from edge 0, one leads forward, the other backward

	const EdgeList expected{ {1, 2}, {2, 3},  {3, 4} };
	expect(recognize_path(graph.begin(), graph.end()), "The input graph is a path");
	expect(equal(graph.begin(), graph.end(), expected.begin(), edges_equal), "The input path is in continuous order");
}

void test_classify_string()
{
	EdgeList edges{ {3, 5}, {4, 3}, {7, 4} }; // path-only caterpillar, no leaves

	const auto result = classify(edges);
	const auto& graph = result.first;

	expect(GraphClass::CATERPILLAR == result.second, "Caterpillar without leaves is correctly classified");
	expect(4 == graph.spines().size(), "All disks are spines");
	expect(0 == graph.branches().size(), "No branch disks");
	expect(0 == graph.leaves().size(), "No leaf disks");
}

void test_classify_caterpillar()
{
	EdgeList edges{ {3, 5}, {4, 3}, {7, 3} }; // caterpillar, 1 spine, 3 leaves

	const auto result = classify(edges);
	const auto& graph = result.first;

	expect(GraphClass::CATERPILLAR == result.second, "Caterpillar is correctly classified");
	expect(1 == graph.spines().size(), "One spine disk");
	expect(3 == graph.spines()[0].id, "Spine disk is 3");
	expect(3 == graph.branches().size(), "3 branch disks");
	expect(0 == graph.leaves().size(), "No leaf disks");
}

void test_classify_lobster()
{
	//   1 -- 5 -.
	//             3 -- 7 -- 8
	//   2 -- 4 -`
	EdgeList edges{ {3, 5}, {1, 5}, {7, 8}, {4, 3}, {4, 2}, {7, 3} }; // lobster, 1 spine, 3 branches, 3 leaves

	const auto result = classify(edges);
	const auto& graph = result.first;

	expect(GraphClass::LOBSTER == result.second, "Lobster is correctly classified");
	expect(1 == graph.spines().size(), "One spine disk");
	expect(3 == graph.spines()[0].id, "Spine disk is 3");
	expect(3 == graph.branches().size(), "3 branch disks");
	expect(3 == graph.leaves().size(), "3 leaf disks");
}

void test_classify_stumped_lobster()
{
	//   1 -- 5 -.
	//            \       .- 8
	//        6 -- 3 -- 7
	//            /       `- 9
	//   2 -- 4 -`
	EdgeList edges{ {3, 6}, {3, 5}, {1, 5}, {7, 8}, {7, 9}, {4, 3}, {4, 2}, {7, 3} }; // lobster, 1 spine, 3 branches, 3 leaves

	const auto result = classify(edges);
	const auto& graph = result.first;

	expect(GraphClass::LOBSTER == result.second, "Lobster with stump is correctly classified");
	expect(1 == graph.spines().size(), "One spine disk");
	expect(3 == graph.spines()[0].id, "Spine disk is 3");
	expect(4 == graph.branches().size(), "4 branch disks");
	expect(4 == graph.leaves().size(), "4 leaf disks");
}

void test_all()
{
	test_Caterpillar_fromText();
	test_edges_from_text();
	test_separate_leaves();
	test_recognize_path();
	test_recognize_path_continuous();
	test_classify_string();
	test_classify_caterpillar();
	test_classify_lobster();
	test_classify_stumped_lobster();
}
