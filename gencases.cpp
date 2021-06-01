#include "graph.h"
#include <string>
#include <fstream>
#include <iostream>

const int MAX_PROBLEM_SIZE = 100; //!< size of instances to generate

/**
 * Generate lobsters with a lot of branches.
 *
 * Example with 5 spines: (numbering)
 *
 *              15  16  17  18  19       < leaves top
 *             5   6   7   8   9         < branches top
 *           0   1   2   3   4           < spine
 *        10  11  12  13  14             < branches bottom
 *      20  21  22  23  24               < leaves bottom
 *
 *     29  30   x   x   x   x   x        <
 *   31  25   x   x   x   x   x  33      < extra branch 1
 * 32  26   x   x   x   x   x  27  34    < extra branch 2 -- extra branch 3
 *        x   x   x   x   x  28  35      < extra branch 4
 *      x   x   x   x   x      36        <
 *
 *      x   x   x   x   x   x   x        <
 *    x   x   x   x   x   x   x   x      <
 *  x   x   x   x   x   x   x   x   x    <
 *   37   x   x   x   x   x   x   x      < extra
 *      x   x   x   x   x  38   x        < leaves
 */
void max_branches(int size)
{
	EdgeList edges;

	for (int j = 0; j < size; j++) {
		if(j > 0) edges.push_back({ j - 1, j }); // spine
		edges.push_back({ size + j, j }); // branches top
		edges.push_back({ 2 * size + j, j }); // branches bottom
		edges.push_back({ 3 * size + j, size + j }); // leaves top
		edges.push_back({ 4 * size + j, 2 * size + j }); // leaves bottom
	}

	const int extras = 5 * size;

	edges.push_back({ extras, 0 }); // start branch 1
	edges.push_back({ extras + 1, 0 }); // start branch 2
	edges.push_back({ extras + 2, size - 1 }); // end branch 3
	edges.push_back({ extras + 3, size - 1 }); // end branch 4

	for (int i = 0; i < 2; i++) {
		edges.push_back({ extras + 4 + i, extras }); // leaves branch 1
		edges.push_back({ extras + 6 + i, extras + 1 }); // leaves branch 2
		edges.push_back({ extras + 8 + i, extras + 2 }); // leaves branch 3
		edges.push_back({ extras + 10 + i, extras + 3 }); // leaves branch 4
	}

	edges.push_back({ extras + 12, 2 * size }); // extra leaf on start branch
	edges.push_back({ extras + 13, 3 * size - 1 }); // extra leaf on end branch

	std::string file = "maxbranches" + std::to_string(size) + ".txt";
	std::ofstream stream{ file };
	edges_to_text(stream, edges);
	stream.close();
}

/**
 * Generate straight-spine lobsters with a lot of leaves.
 *
 * Example with 7 spines: (numbering)
 *
 *     3   2                             < leaves
 *   4   1   0                           < spine 0, branch 1
 *     5   6                             < leaves
 *
 *              10  11                   < leaves
 *     x   x   9   8  12                 < branch 8
 *   x   x   x   7                       < spine 7
 *     x   x
 *
 *               x   x
 *     x   x   x   x   x
 *   x   x   x   x  13                   < spine 13
 *     x   x  15  14  18                 < branch 14
 *              16  17                   < leaves
 *
 *               x   x
 *     x   x   x   x   x
 *   x   x   x   x   x  19               < spine (no branch)
 *     x   x   x   x   x
 *               x   x
 *
 *               x   x      23  24
 *     x   x   x   x   x  22  21  25
 *   x   x   x   x   x   x  20  26       < two more spines
 *     x   x   x   x   x  28  27  31
 *               x   x      29  30
 *
 *               x   x       x   x
 *     x   x   x   x   x   x   x   x  34  35     < leaves
 *   x   x   x   x   x   x   x   x  32  33  36   < last spine with branch
 *     x   x   x   x   x   x   x   x  37  38     < leaves
 *               x   x       x   x
 */
void max_leaves(int size)
{
	EdgeList edges;

	int n = 0; // number of edges added
	int prev = 0; // previous spine id

	// start with constant 5-leaf branch
	{
		int branch = ++n;
		edges.push_back({ prev, branch });

		for (int leaf = 0; leaf < 5; leaf++)
			edges.push_back({ branch, ++n });
	}

	// do spine vertices except first
	for (int j = 1; j < size; j++) {
		int spine = ++n;
		edges.push_back({ prev, spine });
		prev = spine;

		if (j % 3 != 0) { // leave space every 3rd spine for more leaves
			int branch = ++n;
			edges.push_back({ spine, branch });

			for(int leaf = 0; leaf < 4; leaf++)
				edges.push_back({ branch, ++n });
		}
	}

	// add another 5-leaf branch at the end
	{
		int branch = ++n;
		edges.push_back({ prev, branch });

		for (int leaf = 0; leaf < 5; leaf++)
			edges.push_back({ branch, ++n });
	}

	std::string file = "maxleaves" + std::to_string(size) + ".txt";
	std::ofstream stream{ file };
	edges_to_text(stream, edges);
	stream.close();
}

/**
 * Generate lobsters in with every other branch is very heavy.
 * This leads to forced bends if the branches are placed alternatingly.
 *
 * Example with 7 spines: (numbering)
 *
 *     3   2                             < leaves
 *   4   1   0                           < spine 0, branch 1
 *     5   6                             < leaves
 *
 *              10  11                   < leaves
 *     x   x   9   8  12                 < branch 8
 *   x   x   x   7                       < spine 7
 *     x   x
 *
 *               x   x
 *     x   x   x   x   x
 *   x   x   x   x  13                   < spine 13
 *     x   x      14                     < branch 14
 *
 *               x   x  17  18
 *     x   x   x   x   x  16  19         < branch 16, leaves
 *   x   x   x   x   x  15  20           < spine 15
 *     x   x       x
 *
 *               x   x   x   x
 *     x   x   x   x   x   x   x
 *   x   x   x   x   x   x   x  25  26
 *     x   x       x  22  21  23  24  27  < spine 21, 23
 *                                  28
 *
 *               x   x   x   x
 *     x   x   x   x   x   x   x
 *   x   x   x   x   x   x   x   x   x
 *     x   x       x   x   x   x   x   x
 *                          35  29   x   < spine 29
 *                        34  30  31     < branch 30, leaves
 *                          33  32
 */
void onesided_bent(int size)
{
	EdgeList edges;

	int n = 0; // number of edges added
	int prev = 0; // previous spine id

	// start with constant 5-leaf branch
	{
		int branch = ++n;
		edges.push_back({ prev, branch });

		for (int leaf = 0; leaf < 5; leaf++)
			edges.push_back({ branch, ++n });
	}

	// do spine vertices except first
	for (int j = 1; j < size; j++) {
		int spine = ++n;
		edges.push_back({ prev, spine });
		prev = spine;

		if (j % 2 > 0) { // heavy side
			int branch = ++n;
			edges.push_back({ spine, branch });

			for (int leaf = 0; leaf < 4; leaf++)
				edges.push_back({ branch, ++n });
		}
		else { // light side
			edges.push_back({ spine, ++n });
		}
	}

	// add another 5-leaf branch at the end
	{
		int branch = ++n;
		edges.push_back({ prev, branch });

		for (int leaf = 0; leaf < 5; leaf++)
			edges.push_back({ branch, ++n });
	}

	std::string file = "onesided_bent" + std::to_string(size) + ".txt";
	std::ofstream stream{ file };
	edges_to_text(stream, edges);
	stream.close();
}

/**
 * Generate lobsters in with every other branch is very heavy.
 * With strategic placement, these instances can be embedded on a straight spine.
 *
 * Example with 6 spines: (numbering)
 *
 *     3   2                             < leaves
 *   4   1   0                           < spine 0, branch 1
 *     5   6                             < leaves
 *
 *              10  11                   < leaves
 *     x   x   9   8  12                 < branch 8
 *   x   x   x   7                       < spine 7
 *     x   x
 *
 *               x   x
 *     x   x   x   x   x
 *   x   x   x   x  13  15               < spine 13, 15
 *     x   x      14                     < branch 14
 *
 *               x   x      19  20
 *     x   x   x   x   x  18  17  21     < branch 17, leaves
 *   x   x   x   x   x   x  16           < spine 16
 *     x   x       x
 *
 *               x   x       x   x
 *     x   x   x   x   x   x   x   x
 *   x   x   x   x   x   x   x  22  24   < spine 22
 *     x   x       x          28  23  25 < branch 23, leaves
 *                              27  26
 */
void onesided_straight(int size)
{
	EdgeList edges;

	int n = 0; // number of edges added
	int prev = 0; // previous spine id

	// start with constant 5-leaf branch
	{
		int branch = ++n;
		edges.push_back({ prev, branch });

		for (int leaf = 0; leaf < 5; leaf++)
			edges.push_back({ branch, ++n });
	}

	// do spine vertices except first
	for (int j = 1; j < size; j++) {
		int spine = ++n;
		edges.push_back({ prev, spine });
		prev = spine;

		if (j % 3 == 1) { // heavy side
			int branch = ++n;
			edges.push_back({ spine, branch });

			for (int leaf = 0; leaf < 4; leaf++)
				edges.push_back({ branch, ++n });
		}
		else if (j % 3 == 1) { // light side
			edges.push_back({ spine, ++n });
		}
	}

	// add another 5-leaf branch at the end
	{
		int branch = ++n;
		edges.push_back({ prev, branch });

		for (int leaf = 0; leaf < 5; leaf++)
			edges.push_back({ branch, ++n });
	}

	std::string file = "onesided_straight" + std::to_string(size) + ".txt";
	std::ofstream stream{ file };
	edges_to_text(stream, edges);
	stream.close();
}

int main(int argc, const char* argv[])
{
	std::cout << "Generate input graphs up to size " << MAX_PROBLEM_SIZE << ".\n";

	std::cout << "Run generators...\n";
	for (int i = 1; i <= MAX_PROBLEM_SIZE; i++) {
		std::cout << "\tsize " << i << "...\n";
		max_branches(i);
		max_leaves(i);
		onesided_bent(i);
		onesided_straight(i);
	}

	std::cout << "Generators Done.\n";
}
