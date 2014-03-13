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

#include <algorithm>
#include <stdexcept>
#include "types.hpp"

using namespace templet;
using namespace templet::types;

std::string Data::getValue() const {
    throw std::runtime_error("Data item is not of type value");
}

const DataVector& Data::getList() const {
    throw std::runtime_error("Data item is not of type list");
}

DataValue::DataValue(std::string value)
    : _value(std::move(value)) {

}

bool DataValue::empty() const {
    return _value.empty();
}

std::string DataValue::getValue() const {
    return _value;
}

DataList::DataList(DataVector&& data)
    : _data(std::move(data)) {

}

DataList::DataList(std::initializer_list<std::string>&& items)
    : _data() {
    for(auto& item : items) {
        _data.push_back(make_data(std::move(item)));
    }
}

bool DataList::empty() const {
    return _data.empty();
}

const DataVector& DataList::getList() const {
    return _data;
}
