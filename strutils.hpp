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

#ifndef STRUTIL_HPP
#define STRUTIL_HPP

#include <algorithm>
#include <string>

namespace mylib {

static bool starts_with(const std::string& lhs, const std::string& rhs) {
    if(lhs.size() < rhs.size()) {
        return false;
    }
    return std::equal(rhs.cbegin(), rhs.cend(), lhs.begin());
}

static bool ends_with(const std::string& lhs, const std::string& rhs) {
    if(lhs.size() < rhs.size()) {
        return false;
    }
    return std::equal(rhs.crbegin(), rhs.crend(), lhs.crbegin());
}

} // namespace mylib

#endif
