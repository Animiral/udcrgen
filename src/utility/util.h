/**
 * Provides independent, self-contained utility and helper functions.
 */
#pragma once

#include <string>
#include <numeric>
#include <ranges>
#include <type_traits>
#include <iostream>
#include <stdexcept>
#include <cassert>

/**
 * Minimally functional string formatting to substitute for std::format.
 * The standard <format> header is not yet supported.
 */
std::string format(std::string fmt);

template<typename Arg, typename ...Args>
std::string format(std::string fmt, Arg arg, Args... args)
{
    const auto index = fmt.find("{}", 0);
    if (index != std::string::npos) {
        return fmt.substr(0, index) + std::to_string(arg) + format(fmt.substr(index + 2), args...);
    }
    else {
        return fmt;
    }
}

template<typename ...Args>
void trace(std::string fmt, Args... args)
{
    std::cerr << format(fmt, args...) << "\n";
}

/**
 * Join the given string-like objects with the given separator.
 *
 * @param separator: string to appear in-between all others
 * @param strings: range of strings
 * @return: the combined string
 */
template <typename Separator, typename Strings>
std::string join(const Separator& separator, const Strings& strings)
{
    assert(!strings.empty());

    std::string result;
    const auto strSize = std::accumulate(
        std::ranges::cbegin(strings), std::ranges::cend(strings),
        0ull, [](std::size_t sum, const auto& s) { return sum + s.size(); });
    result.reserve((std::size(strings) - 1) * std::size(separator) + strSize + 1);

    result.append(*std::ranges::begin(strings));
    for (const auto& s : std::ranges::subrange(++std::ranges::begin(strings), std::ranges::end(strings))) {
        result.append(separator);
        result.append(s);
    }
    return result;
}

/**
 * Checking narrowing cast for integer types from The C++ Programming Language 11.5.
 */
template<class Target, class Source>
//requires std::is_integral_v<Target>&& std::is_integral_v<Source>
Target narrow_cast(Source v)
{
    auto r = static_cast<Target>(v);
    if (static_cast<Source>(r) != v)
        throw std::domain_error(format("narrow_cast({}) failed.", v));

    return r;
}
