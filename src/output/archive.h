// Output routine for degree files

#include "utility/graph.h"
#include <filesystem>

/**
 * @brief Write graphs to degree files.
 */
class Archive
{

public:

	/**
	 * Set the destination folders for archive files.
	 */
	void setPaths(std::filesystem::path yes, std::filesystem::path no);

	/**
	 * @brief Write the given lobster graph.
	 */
	void write(const Lobster& lobster, bool success) const;

	/**
	 * Return the unique filename for the given lobster instance.
	 *
	 * The filename is simply a concatenation of the lobster's degree notation.
	 * It uses one digit per branch for the leaf count, with 'x' denoting no branch.
	 * Spines are separated by underscores.
	 * The extension is ".txt".
	 */
	static std::filesystem::path fileName(const Lobster& lobster);

private:

	std::filesystem::path yes_; // output base path for successfully layouted instances
	std::filesystem::path no_; // output base path for instances without layout

};
