#include "csv.h"

Csv::Csv(char separator) noexcept
	: stream_(), separator_(separator)
{
}

void Csv::open(const std::filesystem::path& path, std::ios::openmode mode)
{
	if (stream_.is_open())
		stream_.close();

	bool exists = std::filesystem::exists(path);

	stream_.open(path, mode);

	if (!exists || !(mode & std::ios::app))
		header();
}

void Csv::close()
{
	stream_.close();
}

void Csv::write(const Stat& stat)
{
	stream_
		<< Configuration::algorithmString(stat.algorithm) << separator_
		<< stat.size << separator_
		<< stat.spines << separator_
		<< stat.success << separator_
		<< stat.duration.count() << "\n";
}

void Csv::header()
{
	stream_
		<< "Algorithm" << separator_
		<< "Size" << separator_
		<< "Spines" << separator_
		<< "Success" << separator_
		<< "Duration(ms)" << "\n";
}
