/*

The MIT License (MIT)

Copyright (c) 2014 https://github.com/labyrinthofdreams

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#ifndef TRIM_HPP
#define TRIM_HPP

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>

namespace mylib {

/**
 * @brief A functor that checks if a char value is whitespace
 */
struct is_ws {
    /**
     * @brief Check if c is whitespace
     * @param c Value to check
     * @return True if whitespace, otherwise false
     */
    bool operator()(const char c) const {
        return std::isspace(c);
    }
};

template <class InIter, class Cmp = is_ws>
std::pair<InIter, InIter> trim_until(InIter first, InIter last, Cmp ws_cmp = Cmp()) {
    auto new_begin = std::find_if_not(first, last, std::cref(ws_cmp));
    return {new_begin, last};
}

// Trimmed versions do not modify the argument
template <class Cmp = is_ws>
std::string ltrimmed(const std::string& in) {
    auto p = trim_until(in.cbegin(), in.cend(), Cmp());
    return {p.first, p.second};
}

template <class Cmp = is_ws>
std::string rtrimmed(const std::string& in) {
    auto p = trim_until(in.crbegin(), in.crend(), Cmp());
    return {p.second.base(), p.first.base()};
}

template <class Cmp = is_ws>
std::string trimmed(const std::string& in) {
    auto p1 = trim_until(in.cbegin(), in.cend(), Cmp());
    auto p2 = trim_until(in.crbegin(), in.crend(), Cmp());
    return {p1.first, p2.first.base()};
}

// Trim versions modify the argument
// Copied from http://stackoverflow.com/a/217605/110397
static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

static inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}

} // namespace mylib

#endif
