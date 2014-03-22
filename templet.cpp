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
#include <memory>
#include "nodes.hpp"
#include "strutils.hpp"
#include "templet.hpp"
#include "trim.hpp"

using namespace templet;
using namespace templet::nodes;

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
    try {
        reset();
        auto copied = _text;
        _nodes = tokenize(copied);
        for(const auto& node : _nodes) {
            node->evaluate(_parsed, values);
        }
    }
    catch(const templet::exception::InvalidTagError& ex) {
        throw;
    }
    catch(const templet::exception::MissingTagError& ex) {
        throw;
    }
    catch(...) {
        throw;
    }

    return result();
}

std::string Templet::result() const {
    return _parsed.str();
}

std::vector<std::shared_ptr<Node> > Templet::tokenize(std::string &in) try {
    std::vector<std::shared_ptr<Node> > nodes;
    while(!in.empty()) {
        // Parse TEXT until first TAG
        const auto pos = in.find('{');
        if(pos == std::string::npos) {
           if(!in.empty()) {
               // Plain text
               nodes.push_back(std::make_shared<Text>(in));
               in.clear();
           }
           break;
        }
        nodes.push_back(std::make_shared<Text>(in.substr(0, pos)));
        in.substr(pos).swap(in);

        // Find where the tag ends
        const auto end_pos = in.find('}');
        if(end_pos == std::string::npos) {
           // Plain text
           nodes.push_back(std::make_shared<Text>(in));
           in.clear();
           break;
        }

        const auto tag = in.substr(0, end_pos + 1);
        // Parse tag
        if(tag[1] == '\\') {
            // Ignored tag, remove the first \ after opening tag character
            auto new_tag = tag.substr(2);
            new_tag.insert(0, "{");
            nodes.push_back(std::make_shared<Text>(new_tag));
            in.substr(tag.size()).swap(in);
        }
        else if(tag[1] == '$') {
            auto node = ::templet::nodes::parse_value_tag(tag);
            in.substr(tag.size()).swap(in);
            nodes.push_back(std::move(node));
        }
        else if(tag[1] == '%') {
            const auto inner = mylib::ltrimmed(tag.substr(2));
            // adding endif and endfor as nodes it would be possible
            // to check whether an if/for node was closed properly
            // and throw an exception if not
            if(mylib::starts_with(inner, "endif")) {
                in.substr(tag.size()).swap(in);
                break;
            }
            else if(mylib::starts_with(inner, "endfor")) {
                in.substr(tag.size()).swap(in);
                break;
            }
            else if(mylib::starts_with(inner, "if")) {
                auto node = ::templet::nodes::parse_ifvalue_tag(tag);
                in.substr(tag.size()).swap(in);
                node->setChildren(tokenize(in));
                nodes.push_back(std::move(node));
            }
            else if(mylib::starts_with(inner, "elif")) {
                auto node = ::templet::nodes::parse_elifvalue_tag(tag);
                in.substr(tag.size()).swap(in);
                node->setChildren(tokenize(in));
                nodes.push_back(std::move(node));
            }
            else if(mylib::starts_with(inner, "else")) {
                auto node = std::make_shared<ElseValue>();
                in.substr(tag.size()).swap(in);
                node->setChildren(tokenize(in));
                nodes.push_back(std::move(node));
            }
            else if(mylib::starts_with(inner, "for")) {
                auto node = ::templet::nodes::parse_forvalue_tag(tag);
                in.substr(tag.size()).swap(in);
                node->setChildren(tokenize(in));
                nodes.push_back(std::move(node));
            }
            else {
                throw templet::exception::InvalidTagError("Unrecognized tag: " + tag);
            }
        }
        else {
            throw templet::exception::InvalidTagError("Unrecognized tag: " + tag);
        }
    }
    return nodes;
}
catch(const templet::exception::InvalidTagError& ex) {
    throw;
}
catch(...) {
    throw;
}
