// Output unit tests

#include "gtest/gtest.h"
#include "output/archive.h"
#include "utility/graph.h"

TEST(Archive, file_name)
{
	const int NB = Lobster::NO_BRANCH;
	std::vector<Lobster::Spine> spine = {
		{ 3, 2, 2, NB, NB},
		{ 1, NB, NB, NB, NB},
		{ 4, 0, NB, NB, NB}
	};
	Lobster lobster(spine);

	EXPECT_EQ(std::string("322xx_1xxxx_40xxx.txt"), Archive::fileName(lobster).string());
}
