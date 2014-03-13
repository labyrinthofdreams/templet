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

#ifndef SPLIT_HPP
#define SPLIT_HPP

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace mylib {

/**
 * @brief Split a string by a separator
 *
 * Splits a string by a separator and puts the results in iterator
 *
 * @param in String to split
 * @param sep Separator
 * @param out Output iterator
 */
template <class OutIter>
void split(const std::string& in, char sep, OutIter out) {
    if(in.empty()) {
        return;
    }
    std::stringstream ss(in);
    std::string val;
    while(std::getline(ss, val, sep)) {
        *out++ = val;
    }
}

/**
 * @brief Split a string by a separator
 * @param in String to split
 * @param sep Separator
 * @return Parts a vector
 */
std::vector<std::string> split(const std::string& in, char sep) {
    std::vector<std::string> tokens;
    split(in, sep, std::back_inserter(tokens));
    return tokens;
}

} // namespace mylib

#endif
