/**
 * Common definitions for input.
 *
 * @see https://gehrcke.de/2011/06/reading-files-in-c-using-ifstream-dealing-correctly-with-badbit-failbit-eofbit-and-perror/
 */
#pragma once

#include <istream>

/**
 * @brief Read an integer from the given input stream.
 *
 * If the operation is not successful, it may still consume characters from the stream.
 *
 * @param stream source stream
 * @param output[out] integer to write the result to
 * @param minValue ensure that output is greater or equal
 * @param maxValue ensure that output is less or equal
 * @return a reference to the source stream
 * @throw InputException if unsuccessful
 */
std::istream& readint(std::istream& stream, int& output, int minValue = 0, int maxValue = std::numeric_limits<int>::max());

/**
 * @brief Ignore space characters, including the first newline, from the given input stream.
 *
 * Consume the ignored characters and no more.
 * If the operation is not successful, it may still consume characters from the stream.
 *
 * @param stream source stream
 * @return a reference to the source stream
 * @throw InputException if any unexpected non-spaces occur
 */
std::istream& ignoreline(std::istream& stream);
