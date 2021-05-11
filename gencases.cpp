#include "graph.h"
#include <string>
#include <fstream>
#include <iostream>

const int MAX_PROBLEM_SIZE = 100; //!< size of instances to generate

/**
 * Generate lobsters with a lot of branches.
 */
void max_branches()
{
	for (int i = 1; i <= MAX_PROBLEM_SIZE; i++) {
		EdgeList edges;

		for (int j = 0; j < i; j++) {
			if(j > 0) edges.push_back({ j - 1, j }); // spine
			edges.push_back({ i + j, j }); // branches top
			edges.push_back({ 2 * i + j, j }); // branches bottom
			edges.push_back({ 3 * i + j, i + j }); // leaves top
			edges.push_back({ 4 * i + j, 2 * i + j }); // leaves bottom
		}

		const int extras = 5 * i;

		edges.push_back({ extras, 0 }); // start branch 1
		edges.push_back({ extras + 1, 0 }); // start branch 2
		edges.push_back({ extras + 2, i - 1 }); // end branch 3
		edges.push_back({ extras + 3, i - 1 }); // end branch 4

		for (int i = 0; i < 2; i++) {
			edges.push_back({ extras + 4 + i, extras }); // leaves branch 1
			edges.push_back({ extras + 6 + i, extras + 1 }); // leaves branch 2
			edges.push_back({ extras + 8 + i, extras + 2 }); // leaves branch 3
			edges.push_back({ extras + 10 + i, extras + 3 }); // leaves branch 4
		}

		edges.push_back({ extras + 12, 2 * i }); // extra leaf on start branch
		edges.push_back({ extras + 13, 3 * i - 1 }); // extra leaf on end branch

		std::string file = "maxbranches" + std::to_string(i) + ".txt";
		std::ofstream stream{ file };
		edges_to_text(stream, edges);
		stream.close();
	}
}

int main(int argc, const char* argv[])
{
	std::cout << "Generate input graphs up to size " << MAX_PROBLEM_SIZE << ".\n";

	std::cout << "Run generator max_branches...\n";
	max_branches();

	std::cout << "Generator Done.\n";
}
