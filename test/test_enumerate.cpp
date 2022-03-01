// Enumerator unit tests

#include "gtest/gtest.h"
#include "enumerate.h"
#include "heuristic.h"
#include <memory>

/**
 * Test that lobsters are correctly iterated and tested.
 */
TEST(Enumerate, enumerate)
{
	WeakEmbedder fast;
	DynamicProblemEmbedder reference;
	Enumerate enumerate(fast, reference, 0, 10);

	const auto NB = Lobster::NO_BRANCH;
	enumerate.setCurrent({ {{4, 3, 3, NB, NB}, {3, 2, NB, NB, NB}} });
	EXPECT_TRUE(enumerate.test().refSuccess); // embed possible

	enumerate.next(); // success advancement strategy: increase branches
	EXPECT_EQ(enumerate.current(), Lobster({ {4, 3, 3, NB, NB}, {3, 2, 0, NB, NB} }));
	EXPECT_FALSE(enumerate.test().refSuccess); // embed not possible

	enumerate.next(); // failure advancement strategy: increase leaves on prev branch
	EXPECT_EQ(enumerate.current(), Lobster({ {4, 3, 3, NB, NB}, {3, 3, NB, NB, NB} }));
	EXPECT_FALSE(enumerate.test().refSuccess); // embed not possible

	enumerate.next(); // failure advancement strategy: increase leaves on prev branch
	EXPECT_EQ(enumerate.current(), Lobster({ {4, 3, 3, NB, NB}, {4, NB, NB, NB, NB} }));
	EXPECT_TRUE(enumerate.test().refSuccess); // embed possible

	enumerate.next(); // success advancement strategy: increase branches
	EXPECT_EQ(enumerate.current(), Lobster({ {4, 3, 3, NB, NB}, {4, 0, NB, NB, NB} }));
	EXPECT_FALSE(enumerate.test().refSuccess); // embed not possible

	enumerate.next(); // skip past all remaining configs on spine #2, increase branches on prev spine
	EXPECT_EQ(enumerate.current(), Lobster({ {4, 3, 3, 0, NB}, {NB, NB, NB, NB, NB} }));
	EXPECT_TRUE(enumerate.test().refSuccess); // embed possible

	// final lobster of size 2
	enumerate.setCurrent({ {{5, 5, NB, NB, NB}, {NB, NB, NB, NB, NB}} });
	EXPECT_FALSE(enumerate.test().refSuccess); // embed not possible

	enumerate.next(); // failure advancement strategy: increase lobster size
	EXPECT_EQ(enumerate.current(), Lobster({ {NB, NB, NB, NB, NB}, {NB, NB, NB, NB, NB}, {NB, NB, NB, NB, NB} }));
}

/**
 * Test recognizing canonical lobster mirrors.
 */
TEST(Enumerate, isCanonicallyOriented)
{
	const auto NB = Lobster::NO_BRANCH;

	Lobster headHeavy({ {2, 1, NB, NB, NB}, {1, NB, NB, NB, NB}, {1, 1, NB, NB, NB} });
	EXPECT_TRUE(Enumerate::isCanonicallyOriented(headHeavy));

	Lobster tailHeavy({ {5, 5, 5, 5, 4}, {1, NB, NB, NB, NB}, {5, 5, 5, 5, 5} });
	EXPECT_FALSE(Enumerate::isCanonicallyOriented(tailHeavy));

	// palindrome lobsters are always canonical
	Lobster palindrome({ {2, 1, NB, NB, NB}, {1, NB, NB, NB, NB}, {2, 1, NB, NB, NB} });
	EXPECT_TRUE(Enumerate::isCanonicallyOriented(palindrome));
}

/**
 * Test that lobsters are correctly iterated considering canonical orientation.
 */
TEST(Enumerate, nextCanonical)
{
	WeakEmbedder fast;
	DynamicProblemEmbedder reference;
	Enumerate enumerate(fast, reference, 0, 10);
	
	const auto NB = Lobster::NO_BRANCH;

	// The next lobster after a tail-heavy is a palindrome
	enumerate.setCurrent({ {{1, NB, NB, NB, NB}, {1, 1, NB, NB, NB}} });
	enumerate.next();
	EXPECT_EQ(enumerate.current(), Lobster({ {1, 0, NB, NB, NB}, {NB, NB, NB, NB, NB} }));

	// The next lobster after a palindrome is a head-heavy
	enumerate.setCurrent({ {{3, 2, 0, 0, 0}, {3, 2, 0, 0, 0}} });
	enumerate.next();
	EXPECT_EQ(enumerate.current(), Lobster({ {3, 2, 1, 0, 0}, {NB, NB, NB, NB, NB} }));
}
