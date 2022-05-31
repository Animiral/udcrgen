/**
 * Shared definition of benchmark statistics.
 */
#pragma once

#include "config.h"
#include <chrono>

 /**
  * Statistics record of one execution of an embedding algorithm.
  */
struct Stat
{
	Configuration::Algorithm algorithm;
	int size; //!< total number of input vertices
	int spines; //!< number of spine input vertices

	bool success; //!< true if an embedding was determined possible, false otherwise
	std::chrono::microseconds duration; //!< run duration of algorithm
};
