// Various unit tests

#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include "graph.h"

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
void test_edgesFromText()
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
	const auto result = edgesFromText(stream);
	expect(7 == result.size(), "Read 7 edges");
	expect(5 == result.at(0).from, "Edge 0 from 5");
	expect(6 == result.at(1).from, "Edge 1 from 6");
	expect(3 == result.at(1).to, "Edge 1 to 3");
}

bool edgesEqual(Edge e, Edge f)
{
	return e.from == f.from && e.to == f.to;
}

void test_separateLeaves()
{
	EdgeList graph{ {3, 5}, {4, 3}, {7, 4} };

	const EdgeList expected{ {4, 3}, {7, 4}, {3, 5} };
	const auto result = separateLeaves(graph.begin(), graph.end());
	expect(result == graph.begin() + 1, "One non-leaf edge");
	expect(equal(graph.begin(), graph.end(), expected.begin(), edgesEqual), "Leaf edges are removed to the back");
}

void test_recognizePath()
{
	EdgeList graph{ {3, 5}, {4, 3}, {7, 4} };

	const EdgeList expected{ {5, 3}, {3, 4}, {4, 7} };
	expect(recognizePath(graph.begin(), graph.end()), "The input graph is a path");
	expect(equal(graph.begin(), graph.end(), expected.begin(), edgesEqual), "Path edges become aligned");
}

void test_all()
{
	test_Caterpillar_fromText();
	test_edgesFromText();
	test_separateLeaves();
	test_recognizePath();
}
