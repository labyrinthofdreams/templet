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

#include <memory>
#include <stdexcept>
#include "nodes.hpp"
#include "split.hpp"

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

}

void Value::evaluate(std::ostream& os, DataMap& kv) const {
    const bool has_dot = _name.find('.') != std::string::npos;
    if(has_dot) {
        auto tks = mylib::split(_name, '.');
        if(tks.size() > 1) {
        }
    }
    else {
        try {
            if(kv.count(_name)) {
                os << kv[_name]->getValue();
            }
        }
        catch(const std::exception &ex) {
        }
    }
}

IfValue::IfValue(std::string name)
    : Node(), _name(std::move(name)), _nodes() {

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
