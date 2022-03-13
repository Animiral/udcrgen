// Configuration unit tests

#include "gtest/gtest.h"
#include "config.h"
#include "utility/exception.h"

TEST(Config, validate)
{
	Configuration configuration;
	EXPECT_THROW(configuration.validate(), ConfigException);
	configuration.inputFile = "foo";
	EXPECT_NO_THROW(configuration.validate());

	configuration.algorithm = Configuration::Algorithm::BENCHMARK;
	configuration.inputFile = "foo";
	EXPECT_THROW(configuration.validate(), ConfigException);

	configuration = {}; // reset
	configuration.spineMin = configuration.spineMax = 10;
	EXPECT_THROW(configuration.validate(), ConfigException);
}
