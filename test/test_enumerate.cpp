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
		// TODO
	}

};

/**
 * Test that lobsters are correctly iterated.
 */
TEST(Enumerate, next)
{
	// TODO
}

/**
 * Test that the algorithms are properly run.
 */
TEST(Enumerate, test)
{
	// TODO
}
