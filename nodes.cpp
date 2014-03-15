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
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>
#include "nodes.hpp"
#include "ptrutil.hpp"
#include "split.hpp"
#include "strutils.hpp"
#include "trim.hpp"

using namespace templet::nodes;

void Node::setChildren(std::vector<std::unique_ptr<Node>>&&) {
    throw std::runtime_error("This Node type cannot have children");
}

Text::Text(std::string text)
    : Node(), _in(std::move(text)) {

}

void Text::evaluate(std::ostream& os, DataMap& /*kv*/) const {
    os << _in;
}

Value::Value(std::string name)
    : Node(), _name(std::move(name)) {
    if(!isValidTag(_name)) {
        throw templet::exception::InvalidTagError("Tag name must only contain a-zA-Z0-9_-");
    }
}

void Value::evaluate(std::ostream& os, DataMap& kv) const {
    try {
        if(kv.count(_name)) {
            os << kv[_name]->getValue();
        }
    }
    catch(const std::exception &ex) {
    }
}

bool Value::isValidTag(const std::string& tagName) const {
    if(tagName.empty()) {
        return false;
    }

    // Tag names may only contain a-zA-Z0-9_-
    auto validator = [](const char c){ return (c >= 'a' && c <= 'z') ||
                                                (c >= 'A' && c <= 'Z') ||
                                                (c >= '0' && c <= '9') ||
                                                (c == '_' || c == '-' || c == '.'); };
    return std::all_of(tagName.cbegin(), tagName.cend(), std::cref(validator));
}

IfValue::IfValue(std::string name)
    : Node(), _name(std::move(name)), _nodes() {
    if(!isValidTag(_name)) {
        throw templet::exception::InvalidTagError("Tag name must only contain a-zA-Z0-9_-");
    }
}

void IfValue::setChildren(std::vector<std::unique_ptr<Node>>&& children) {
    _nodes.swap(children);
}

void IfValue::evaluate(std::ostream& os, DataMap& kv) const {
    // Check that the IF condition is TRUE (it's enough that it's been set)
    if(kv.count(_name)) {
        for(auto& node : _nodes) {
            node->evaluate(os, kv);
        }
    }
}

bool IfValue::isValidTag(const std::string &tagName) const {
    if(tagName.empty()) {
        return false;
    }

    // Tag names may only contain a-zA-Z0-9_-
    auto validator = [](const char c){ return (c >= 'a' && c <= 'z') ||
                                                (c >= 'A' && c <= 'Z') ||
                                                (c >= '0' && c <= '9') ||
                                                (c == '_' || c == '-' || c == '.'); };
    return std::all_of(tagName.cbegin(), tagName.cend(), std::cref(validator));
}

std::unique_ptr<Node> templet::nodes::parse_value_tag(std::string in) {
    if(!mylib::starts_with(in, "{$") || !mylib::ends_with(in, "}")) {
        throw templet::exception::InvalidTagError("Tag must be enclosed with {$ and }");
    }

    in.erase(0, 2);
    const auto arr_pos = in.find('[');
    if(arr_pos != std::string::npos) {
        in.erase(arr_pos);
    }
    else {
        in.erase(in.find('}'));
    }
    in = mylib::trim(in);
    return mylib::make_unique<Value>(std::move(in));
}

std::unique_ptr<Node> templet::nodes::parse_ifvalue_tag(std::string in) {
    if(!mylib::starts_with(in, "{%") || !mylib::ends_with(in, "%}")) {
        throw templet::exception::InvalidTagError("Tag must be enclosed with {% and %}");
    }

    in.erase(0, 2);
    in.erase(in.find('%'));
    in = mylib::trim(in);
    if(!mylib::starts_with(in, "if ")) {
        throw templet::exception::InvalidTagError("Tag must be prefixed with 'if '");
    }

    in = mylib::ltrim(in.erase(0, 3));

    // TODO: Validate the variable name either via IfValue::is_valid_tag()
    // or in the IfValue constructor
    return mylib::make_unique<IfValue>(std::move(in));
}
