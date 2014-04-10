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
#include <cstddef>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <utility>
#include "nodes.hpp"
#include "split.hpp"
#include "strutils.hpp"
#include "trim.hpp"

using namespace templet::nodes;

namespace {

bool isValidName(const std::string& name) {
    const auto genericNameValidator = [](const char c){
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') || (c == '_' || c == '-');
    };

    return std::all_of(name.cbegin(), name.cend(), std::cref(genericNameValidator));
}

bool isValidNameExpression(const std::string& name) {
    const auto specialNameValidator = [](const char c){
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') || (c == '_' || c == '-') ||
                (c == '[' || c == ']' || c == '.');
    };

    const bool hasMultiDot = name.find("..") != std::string::npos;
    const bool isValidSpecial = std::all_of(name.cbegin(), name.cend(), std::cref(specialNameValidator));

    return isValidSpecial && !hasMultiDot;
}

/**
 * @brief Parse a string into a number
 * @param text String to parse
 * @param results Save the parsed integer
 * @return True on success, otherwise false
 */
bool parse_number(const std::string& text, int& result) {
    std::istringstream parser {text};
    return parser >> result >> std::ws && parser.peek() == EOF;
}

/**
 * @brief Parse an array index
 *
 * Given a string like [5], returns 5
 *
 * Negative indexes are allowed
 *
 * @param in String to parse
 * @exception templet::exception::InvalidTagError
 * @return Index as an int
 */
int parse_array_index(std::string in) {
    if(!mylib::starts_with(in, "[") || !mylib::ends_with(in, "]")) {
        throw templet::exception::InvalidTagError("Invalid array syntax: Value must be enclosed with []");
    }

    int idx = 0;
    const auto raw_number = in.substr(1, in.size() - 2);
    const bool is_number = parse_number(raw_number, idx);
    if(!is_number) {
        throw templet::exception::InvalidTagError("Invalid array index: Value must be an integer");
    }

    return idx;
}

/**
 * @brief Parse a single tag name with optional array index
 *
 * Given a string config[5] returns pair("config", 5)
 *
 * @param in Tag name to parse as a string
 * @exception templet::exception::InvalidTagError if the index is negative
 * or if parse_array_index throws
 * @return Tag name and index value as a pair
 */
std::pair<std::string, int> parse_tag_name(std::string in) {
    // TODO: Add name validation here (e.g. checkibng final ])?
    const auto arr_pos = in.find('[');
    if(arr_pos == std::string::npos) {
        return {std::move(in), -1};
    }

    const int idx = parse_array_index(in.substr(arr_pos));
    if(idx < 0) {
        throw templet::exception::InvalidTagError("Invalid array index: Value must not be negative");
    }
    in.erase(arr_pos);

    return {std::move(in), idx};
}

/**
 * @brief Parse a full tag name
 *
 * Parses a full tag name, e.g.: config.servers[1].users[6].username
 *
 * @param name String to parse
 * @param kv Map of values to reference
 * @exception templet::exception::MissingTagError If tag is not found in kv
 * @exception templet::exception::InvalidTagError
 * @return DataPtr to the parsed tag value
 */
templet::types::DataPtr parse_tag(std::string name, const DataMap& kv) {
    // mapRef holds a reference to the current map level in dot notated tags
    auto mapRef = std::cref(kv);
    // lastItem holds a pointer to the last evaluated value in the tag
    templet::types::DataPtr lastItem;
    while(!name.empty()) {
        const auto pos = name.find('.');
        // Parse the first found tag name which may contain [n]...[n]
        const auto tag = name.substr(0, pos);
        if(pos == std::string::npos) {
            name.clear();
        }
        else {
            name.erase(0, pos + 1);
        }
        const auto arrPos = tag.find('[');
        // tagName contains the name before list indexing [n]...
        const auto tagName = tag.substr(0, arrPos);
        if(tagName.empty()) {
            // This is true when e.g.:
            // {$ config.[1] } {$ .username }
            throw templet::exception::InvalidTagError("Tags must have a name: " + tag);
        }
        else if(!isValidName(tagName)) {
            throw templet::exception::InvalidTagError("Invalid syntax: " + tag);
        }
        const auto& map = mapRef.get();
        if(!map.count(tagName)) {
            //throw templet::exception::MissingTagError("Tag name not found: " + tagName);
            lastItem.reset();
            break;
        }
        lastItem = map.at(tagName);
        // Parse the array index syntax
        if(arrPos != std::string::npos) {
            if(lastItem->type() != templet::types::DataType::List) {
                //throw templet::exception::InvalidTagError("Array syntax can only be used to access elements in lists");
                lastItem.reset();
                break;
            }
            auto listRef = std::cref(lastItem->getList());
            auto arr = tag.substr(arrPos);
            while(!arr.empty()) {
                const auto arrEndPos = arr.find(']');
                const std::size_t index = parse_array_index(arr.substr(0, arrEndPos + 1));
                arr.erase(0, arrEndPos + 1);
                if(!arr.empty() && arr[0] != '[') {
                    // Valid e.g. for groups[0]users[1]
                    throw templet::exception::InvalidTagError("Invalid syntax: " + tag);
                }
                if(index >= listRef.get().size()) {
                    //throw templet::exception::InvalidTagError("Array index out of bounds: " + tag);
                    name.clear();
                    lastItem.reset();
                    break;
                }
                lastItem = listRef.get().at(index);
                if(lastItem->type() != templet::types::DataType::List) {
                    // All elements accessed via the array index sequence [n]...[m]
                    // must be lists, with the exception of the last element
                    // which may be a string, a map, or a list
                    if(arr.empty()) {
                        break;
                    }
                    //throw templet::exception::InvalidTagError("Array syntax can only be used to access elements in lists");
                    name.clear();
                    lastItem.reset();
                    break;
                }
                listRef = std::cref(lastItem->getList());
            }
        }
        // After the tag name has been parsed and evaluated, if the remaining
        // tag name is not empty (implying dot notation), the last evaluated
        // item must be a map value
        if(!name.empty()) {
            if(lastItem->type() != templet::types::DataType::Mapper) {
                throw templet::exception::InvalidTagError("Dot notation can only be used on maps");
            }

            mapRef = std::cref(lastItem->getMap());
        }
    }
    return lastItem;
}

/**
 * @brief A helper function for \link parse_tag \endlink that evaluates into a string
 * @param name Tag name to parse
 * @param kv Values to reference
 * @exception templet::exception::InvalidTagError if result is not a string
 * @return Parsed result as a string
 */
std::string parse_tag_string(const std::string& name, const DataMap& kv) {
    const auto& res = parse_tag(name, kv);
    if(!res) {
            throw templet::exception::MissingTagError("Tag name not found: " + name);
    }
    else if(res->type() != templet::types::DataType::String) {
        throw templet::exception::InvalidTagError("Invalid tag name: Name must reference a string");
    }

    return res->getValue();
}

/**
 * @brief A helper function for \link parse_tag \endlink that evaluates into a vector
 * @param name Tag name to parse
 * @param kv Values to reference
 * @exception templet::exception::InvalidTagError if result is not a vector
 * @return Parsed result as a vector
 */
const templet::types::DataVector& parse_tag_list(const std::string& name, const DataMap& kv) {
    const auto& res = parse_tag(name, kv);
    if(!res) {
            throw templet::exception::MissingTagError("Tag name not found: " + name);
    }
    else if(res->type() != templet::types::DataType::List) {
        throw templet::exception::InvalidTagError("Invalid tag name: Name must reference a list");
    }

    return res->getList();
}

} // unnamed namespace

void Node::setChildren(std::vector<std::shared_ptr<Node>> /*children*/) {
    throw std::runtime_error("This Node type cannot have children");
}

NodeType Node::type() const {
    return NodeType::Invalid;
}

Text::Text(std::string text)
    : Node(), _in(std::move(text)) {

}

void Text::evaluate(std::ostream& os, const DataMap& /*kv*/) const {
    os << _in;
}

NodeType Text::type() const {
    return NodeType::Text;
}

Value::Value(std::string name)
    : Node(), _name(std::move(name)) {
    if(!isValidNameExpression(_name)) {
        throw templet::exception::InvalidTagError("Variable tag name contains invalid characters");
    }
}

void Value::evaluate(std::ostream& os, const DataMap& kv) const {
    try {
        os << parse_tag_string(_name, kv);
    }
    catch(const templet::exception::MissingTagError& ex) {
        // Default behavior is to just ignore it, effectively
        // just removing the tag name from the output
        // TODO: Add user config option to re-throw
    }
    catch(...) {
        throw;
    }
}

NodeType Value::type() const {
    return NodeType::Value;
}

IfValue::IfValue(std::string name)
    : Node(), _name(std::move(name)), _nodes() {
    if(!isValidNameExpression(_name)) {
        throw templet::exception::InvalidTagError("If expression tag name contains invalid characters");
    }
}

void IfValue::setChildren(std::vector<std::shared_ptr<Node>> children) {
    _nodes.swap(children);
}

void IfValue::evaluate(std::ostream& os, const DataMap& kv) const {
    // Check that the IF condition is TRUE (it's enough that it's been set)
    auto parsed_tag = parse_tag(_name, kv);
    if(parsed_tag) {
        for(auto& node : _nodes) {
            if(node->type() == templet::nodes::NodeType::ElifValue ||
                    node->type() == templet::nodes::NodeType::ElseValue) {
                break;
            }
            node->evaluate(os, kv);
        }
    }
    else {
        // Do Elif/else block
        for(auto& node : _nodes) {
            if(node->type() == templet::nodes::NodeType::ElifValue ||
                    node->type() == templet::nodes::NodeType::ElseValue) {
                node->evaluate(os, kv);
            }
        }
    }
}

NodeType IfValue::type() const {
    return NodeType::IfValue;
}

ElifValue::ElifValue(std::string name)
    : IfValue(std::move(name)) {

}

NodeType ElifValue::type() const {
    return NodeType::ElifValue;
}

void ElseValue::setChildren(std::vector<std::shared_ptr<Node> > children) {
    _nodes.swap(children);
}

void ElseValue::evaluate(std::ostream& os, const templet::types::DataMap& kv) const {
    for(auto& node : _nodes) {
        node->evaluate(os, kv);
    }
}

NodeType ElseValue::type() const {
    return NodeType::ElseValue;
}


ForValue::ForValue(std::string name, std::string alias)
    : Node(), _name(std::move(name)), _alias(std::move(alias)), _nodes() {
    // Validate names
    if(!isValidNameExpression(_name)) {
        throw templet::exception::InvalidTagError("For expression first tag name contains invalid characters");
    }
    else if(!isValidName(_alias)) {
        throw templet::exception::InvalidTagError("For expression second tag name contains invalid characters");
    }
}

void ForValue::setChildren(std::vector<std::shared_ptr<Node>> children) {
    _nodes.swap(children);
}

void ForValue::evaluate(std::ostream& os, const templet::types::DataMap& kv) const {
    const auto& evaluatedList = parse_tag_list(_name, kv);
    if(kv.count(_alias)) {
        throw templet::exception::InvalidTagError("For expression alias name collides with an existing name");
    }
    // In a for statement the map entries are copied
    // and the 'as' values are added with the new name
    templet::types::DataMap newValues = kv;
    for(const auto& item : evaluatedList) {
        newValues[_alias] = item;
        for(auto& node : _nodes) {
            node->evaluate(os, newValues);
        }
    }
}

NodeType ForValue::type() const {
    return NodeType::ForValue;
}

std::shared_ptr<Node> templet::nodes::parse_value_tag(std::string in) {
    if(!mylib::starts_with(in, "{$") || !mylib::ends_with(in, "}")) {
        throw templet::exception::InvalidTagError("Tag must be enclosed with {$ and }");
    }

    // Remove surrounding {$ and }
    in.erase(0, 2);
    in.erase(in.find('}'));
    in = mylib::trim(in);

    return std::make_shared<Value>(std::move(in));
}

std::shared_ptr<Node> templet::nodes::parse_ifvalue_tag(std::string in) {
    if(!mylib::starts_with(in, "{%") || !mylib::ends_with(in, "%}")) {
        throw templet::exception::InvalidTagError("Tag must be enclosed with {% and %}");
    }

    in.erase(0, 2);
    in.erase(in.find('%'));
    in = mylib::trim(in);
    if(!mylib::starts_with(in, "if ")) {
        throw templet::exception::InvalidTagError("Tag must be prefixed with 'if '");
    }

    in = mylib::ltrim(in.erase(0, in.find(' ') + 1));

    return std::make_shared<IfValue>(std::move(in));
}

std::shared_ptr<Node> templet::nodes::parse_elifvalue_tag(std::string in) {
    if(!mylib::starts_with(in, "{%") || !mylib::ends_with(in, "%}")) {
        throw templet::exception::InvalidTagError("Tag must be enclosed with {% and %}");
    }

    in.erase(0, 2);
    in.erase(in.find('%'));
    in = mylib::trim(in);
    if(!mylib::starts_with(in, "elif ")) {
        throw templet::exception::InvalidTagError("Tag must be prefixed with 'elif '");
    }

    in = mylib::ltrim(in.erase(0, in.find(' ') + 1));

    return std::make_shared<ElifValue>(std::move(in));
}

std::shared_ptr<Node> templet::nodes::parse_forvalue_tag(std::string in)
{
    if(!mylib::starts_with(in, "{%") || !mylib::ends_with(in, "%}")) {
        throw templet::exception::InvalidTagError("Tag must be enclosed with {% and %}");
    }

    in.erase(0, 2);
    in.erase(in.find('%'));
    in = mylib::trim(in);
    auto tokens = mylib::split(in, ' ');
    if(tokens.size() != 4) {
        throw templet::exception::ExpressionSyntaxError("Unrecognized for expression syntax");
    }

    if(tokens[0] != "for" || tokens[2] != "as") {
        throw templet::exception::ExpressionSyntaxError("Unrecognized for expression syntax");
    }

    return std::make_shared<ForValue>(tokens[1], tokens[3]);
}
