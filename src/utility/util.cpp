#include <string>
#include "util.h"

std::ostream& format(std::ostream& stream, const std::string& fmt)
{
    stream << fmt;
    return stream;
}
