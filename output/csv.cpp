#include "csv.h"

Csv::Csv(std::ostream& stream, char separator) noexcept
	: stream_(&stream), separator_(separator)
{
}

void Csv::header()
{
	*stream_
		<< "Algorithm" << separator_
		<< "Size" << separator_
		<< "Spines" << separator_
		<< "Success" << separator_
		<< "Duration(ms)" << "\n";
}

void Csv::write(const Stat& stat)
{
	*stream_
		<< Configuration::algorithmString(stat.algorithm) << separator_
		<< stat.size << separator_
		<< stat.spines << separator_
		<< stat.success << separator_
		<< stat.duration.count() << "\n";
}
