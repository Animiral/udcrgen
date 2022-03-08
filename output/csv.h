// Output routine for CSV stats files

#pragma once

#include "utility/stat.h"
#include <fstream>

/**
 * @brief Write stats to CSV.
 */
class Csv
{

public:

	/**
	 * @brief Construct the CSV object.
	 */
	explicit Csv(char separator = ',') noexcept;

	/**
	 * @brief Open the given CSV file in the given mode.
	 */
	void open(const std::filesystem::path& path, std::ios::openmode mode = std::ios::app);

	/**
	 * @brief Explicitly close this @c Csv's output file.
	 */
	void close();

	/**
	 * @brief Write the given stat line.
	 */
	void write(const Stat& stat);

private:

	std::fstream stream_;
	char separator_;

	void header();

};
