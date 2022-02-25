// Enumerator unit tests

#include "gtest/gtest.h"
#include "enumerate.h"

// helper for testing enumerate behavior
class MockFastEmbedder : public Embedder
{

public:

	virtual void embed(Disk& disk) override
	{
		// TODO
	}

};

// helper for testing enumerate behavior
class MockWholesaleEmbedder : public WholesaleEmbedder
{

public:

	virtual bool embed(std::vector<Disk>& disks) override
	{
		return true;
	}

};

// helper to compare lobster values
bool equal(const Lobster& lhs, const Lobster& rhs) noexcept
{
	auto& ls = lhs.spine();
	auto& rs = rhs.spine();

	return ls == rs;

	//if (ls.size() != rhs.countSpine())
	//	return false;

	//for (int i = 0; i < lhs.spine().size(); i++) {
	//	for (int j = 0; j < 5; j++) {
	//		if (lhs.spine()[i] != rhs.spine()[i])
	//			return false;
	//	}
	//}

	//return true;
}

/**
 * Test that lobsters are correctly iterated.
 */
TEST(Enumerate, next)
{
	auto fast = std::make_unique<MockFastEmbedder>();
	auto reference = std::make_unique <MockWholesaleEmbedder>();

	Enumerate enumerate(*fast, *reference, 0, 10);

	Lobster current{ {{3, 3, 1, 0}, {3, 3, 1, 0}} }; // embed possible
	enumerate.test(current);
	EXPECT_TRUE(enumerate.stats().back().success);

	enumerate.next(); // first advancement strategy: increase leaves on last branch

}

/**
 * Test that the algorithms are properly run.
 */
TEST(Enumerate, test)
{
	// TODO
}
