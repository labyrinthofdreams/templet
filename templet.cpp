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
#include "nodes.hpp"
#include "ptrutil.hpp"
#include "templet.hpp"
#include "trim.hpp"

using namespace templet;
using namespace templet::nodes;
using mylib::make_unique;

Templet::Templet(std::string text)
    : _text(std::move(text)),
      _parsed(),
      _nodes()
{}

void Templet::reset() {
    _parsed.str("");
    _nodes.clear();
}

void Templet::setTemplate(std::string str) {
    _text.swap(str);
    reset();
}

std::string Templet::parse(DataMap &values) {
    reset();
    auto copied = _text;
    _nodes = tokenize(copied);
    for(const auto& node : _nodes) {
        node->evaluate(_parsed, values);
    }

    return result();
}

std::string Templet::result() const {
    return _parsed.str();
}

std::vector<std::unique_ptr<Node> > Templet::tokenize(std::string &in) {
    std::vector<std::unique_ptr<Node> > nodes;
    while(!in.empty()) {
        // Parse TEXT until first TAG
        const auto pos = in.find('{');
        if(pos == std::string::npos) {
           if(!in.empty()) {
               // Plain text
               nodes.push_back(make_unique<Text>(in));
           }
           break;
        }
        nodes.push_back(make_unique<Text>(in.substr(0, pos)));
        in.substr(pos).swap(in);

        // Find where the tag ends
        const auto end_pos = in.find('}');
        if(pos == std::string::npos) {
           // Plain text
           nodes.push_back(make_unique<Text>(in));
           break;
        }

        const auto tag = in.substr(0, end_pos);

        // Inside the TAG
        if(in[0] == '$') {
           // {$ implies the opening of a variable (Value)
           const auto cpos = in.find('}');
           if(cpos != std::string::npos) {
               nodes.push_back(make_unique<Value>(mylib::trimmed(in.substr(1, cpos - 1))));
               in.substr(cpos + 1).swap(in);
           }
           else {
               // Couldn't find }
               nodes.push_back(make_unique<Text>("{"));
               nodes.push_back(make_unique<Text>(in));
               in.clear();
           }
        }
        else if(in[0] == '%') {
           const auto cpos = in.find('}');
           if(cpos != std::string::npos) {
               // expr is {% whateverishere }
               auto expr = mylib::trimmed(in.substr(1, cpos - 2));
               const auto if_pos = expr.find("if");
               if(if_pos != std::string::npos && if_pos == 0) {
                   expr = mylib::ltrimmed(expr.substr(2));
                   in.substr(cpos + 1).swap(in);
                   auto val = make_unique<IfValue>(expr);
                   val->setChildren(tokenize(in));
                   nodes.push_back(std::move(val));
                   continue;
               }
               const auto endif_pos = expr.find("endif");
               if(endif_pos != std::string::npos && endif_pos == 0) {
                   in.substr(cpos + 1).swap(in);
                   break;
               }
           }
           else {
               // Couldn't find }
               nodes.push_back(make_unique<Text>("{"));
               nodes.push_back(make_unique<Text>(in));
               in.clear();
           }
        }
        else {
           nodes.push_back(make_unique<Text>("{"));
        }
    }
    return nodes;
}
