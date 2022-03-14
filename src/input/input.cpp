#include "input.h"
#include "utility/exception.h"
#include <string>
#include <cassert>

std::istream& readint(std::istream& stream, int& output, int minValue, int maxValue)
{
    assert(minValue <= maxValue);

    int o = minValue;
    stream >> o;

    if (stream.bad()) {
        throw InputException(std::strerror(errno));
    }
    if (stream.fail() && !stream.eof()) {
        auto backup = stream.rdstate();
        stream.clear();
        std::string t; // find out what content is there where a number should be
        stream >> t;
        if (stream.bad()) { // stream went sour while examining content
            throw InputException(std::strerror(errno));
        }
        stream.setstate(backup);
        throw InputException("Failed to read degree number.", "", t);
    }

    using namespace std::string_literals;
    if (o < minValue) {
        throw InputException(format("Read value {} is less than min value {}."s, o, minValue));
    }
    if (o > maxValue) {
        throw InputException(format("Read value {} is greater than max value {}."s, o, maxValue));
    }

    output = o;
    return stream;
}

std::istream& ignoreline(std::istream& stream)
{
    for (char sp; stream.get(sp);) {
        if (std::isspace(sp)) { // swallow spaces
            if ('\n' == sp)
                break;
        }
        else {
            stream.unget();
            std::string t; // find out what content is there where a newline should be
            stream >> t;
            throw InputException("Expected new line.", "", t);
        }
    }

    if (stream.bad()) {
        throw InputException(std::strerror(errno));
    }

    return stream;
}
