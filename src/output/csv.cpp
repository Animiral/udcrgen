#include "csv.h"
#include <cstring>
#include "utility/exception.h"

Csv::Csv(char separator) noexcept
	: stream_(), separator_(separator)
{
}

void Csv::open(const std::filesystem::path& path, std::ios::openmode mode)
{
	if (stream_.is_open()) {
		stream_.clear();
		stream_.close();

		if (stream_.fail())
			throw OutputException(std::strerror(errno));
	}

	bool exists = std::filesystem::exists(path);

	stream_.open(path, mode);

	if (!stream_.is_open())
		throw OutputException(std::strerror(errno), path.string());

	if (!exists || !(mode & std::ios::app))
		header();
}

void Csv::close()
{
	stream_.clear();
	stream_.close();

	if (stream_.fail())
		throw OutputException(std::strerror(errno));
}

void Csv::write(const Stat& stat)
{
	stream_
		<< Configuration::algorithmString(stat.algorithm) << separator_
		<< stat.size << separator_
		<< stat.spines << separator_
		<< stat.success << separator_
		<< stat.duration.count() << "\n";

	if (stream_.fail())
		throw OutputException(std::strerror(errno));
}

void Csv::header()
{
	stream_
		<< "Algorithm" << separator_
		<< "Size" << separator_
		<< "Spines" << separator_
		<< "Success" << separator_
		<< "Duration(usec)" << "\n";

	if (stream_.fail())
		throw OutputException(std::strerror(errno));
}
