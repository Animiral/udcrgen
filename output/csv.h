// Output routine for CSV stats files

#pragma once

#include "enumerate.h"
#include <ostream>

/**
 * @brief Write stats to CSV.
 */
class Csv
{

public:

	/**
	 * @brief Construct the CSV object for writing to the given stream.
	 */
	explicit Csv(std::ostream& stream, char separator) noexcept;

	/**
	 * @brief Write the header line.
	 */
	void header();

	/**
	 * @brief Write the given stat line.
	 */
	void write(const Stat& stat);

private:

	std::ostream* stream_;
	char separator_;

};
