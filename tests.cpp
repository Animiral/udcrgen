// Various unit tests

#include <iostream>
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
 * Ensure that we can convert a text representation to a DiskGraph.
 */
void test_UdcrGraph_fromText()
{
	// Each line is of the form
	// <id> <parent>
	// where id is the number of the new vertex and parent is the id of the
	// parent vertex or any arbitrary number in the case of the first vertex.
	const std::string input =
		"3 -1\n"
		"5 3\n"
		"6 3\n"
		"9 3\n"
		"4 3\n"
		"7 4\n"
		"8 4\n"
		"11 8\n";
	std::istringstream stream{ input };
	const auto result = DiskGraph::fromText(stream);
	expect(0 == result.spine(), "Spine is undefined in raw graph");
	expect(8 == result.disks().size(), "Graph has 8 vertices");
	expect(3 == result.disks().at(0).id, "Vertex 0 id is 3");
	expect(5 == result.disks().at(1).id, "Vertex 1 id is 5");
	expect(3 == result.disks().at(1).parent, "Vertex parent of 5 is 3");
}

void test_all()
{
	test_Caterpillar_fromText();
	test_UdcrGraph_fromText();
}
