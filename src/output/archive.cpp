#include "archive.h"
#include <fstream>
#include <cassert>

void Archive::setPaths(std::filesystem::path yes, std::filesystem::path no)
{
	yes_ = yes;
	no_ = no;
}

void Archive::write(const Lobster& lobster, bool success) const
{
	std::filesystem::path outpath = success ? yes_ : no_;

	if (outpath.empty())
		return; // ignore

	outpath /= std::to_string(lobster.countSpine());
	bool dir_created = std::filesystem::create_directories(outpath);
	outpath /= fileName(lobster);
	std::ofstream outfile(outpath, std::ios::out | std::ios::trunc);

	for (const Lobster::Spine& spine : lobster.spine()) {
		outfile << spine[0] << " " << spine[1] << " " << spine[2] << " "
			<< spine[3] << " " << spine[4] << "\n";
	}

	outfile.close();
}

std::filesystem::path Archive::fileName(const Lobster& lobster)
{
	return lobster.identifier() + ".txt";
}
