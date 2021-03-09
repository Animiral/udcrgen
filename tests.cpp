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

void test_all()
{
	test_Caterpillar_fromText();
}
