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

#ifndef NODES_HPP
#define NODES_HPP

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>
#include "types.hpp"

namespace templet {
namespace exception {

/**
 * @brief The invalid tag error is a generic error type
 */
class InvalidTagError : public std::runtime_error {
public:
    InvalidTagError(const char* reason) : std::runtime_error(reason) {}
    InvalidTagError(const std::string& reason) : std::runtime_error(reason) {}
};

/**
 * @brief The missing tag error is thrown when a referenced name
 * in a template tag is not found in the map of values
 */
class MissingTagError : public std::runtime_error {
public:
    MissingTagError(const char* reason) : std::runtime_error(reason) {}
    MissingTagError(const std::string& reason) : std::runtime_error(reason) {}
};

/**
 * @brief The expression syntax error is thrown when there's an error
 * in parsing tag expressions (e.g. if, for)
 */
class ExpressionSyntaxError : public std::runtime_error {
public:
    ExpressionSyntaxError(const char* reason) : std::runtime_error(reason) {}
    ExpressionSyntaxError(const std::string& reason) : std::runtime_error(reason) {}
};

} // namespace exception

namespace nodes {

using ::templet::types::DataMap;

/**
 * @brief The NodeType enum describes what the node represents
 */
enum class NodeType {
    Invalid,    ///< Base Node class
    Text,       ///< A text block
    Value,      ///< A variable block
    IfValue,    ///< An if block
    ElifValue,  ///< An elif block
    ElseValue,  ///< An else block
    ForValue    ///< A for loop block
};

/**
 * @brief The Node class represents a block from the template
 */
class Node {
public:
    Node() = default;

    virtual ~Node() = default;

    /**
     * @brief Set child nodes for this node type
     *
     * By-value parameter leaves the option for user to copy/move the children
     *
     * @exception std::runtime_error If called on a node that doesn't support children
     */
    virtual void setChildren(std::vector<std::shared_ptr<Node>> /*newNodes*/);

    /**
     * @brief Evaluates the Node and outputs the computed value in ostream os
     */
    virtual void evaluate(std::ostream& /*os*/, const DataMap& /*kv*/) const = 0;

    virtual NodeType type() const;
};

/**
 * @brief The Text class represents a plain text block in the template
 */
class Text : public Node {
private:
    std::string _in;

public:
    Text() = default;

    /**
     * @brief Construct a text node with text
     * @param text Text block from the template
     */
    Text(std::string text);

    void evaluate(std::ostream& os, const DataMap& /*kv*/) const override;

    NodeType type() const override;
};

/**
 * @brief The Value class represents a variable in the template
 *
 * Variables can be replaced by another value in the output
 */
class Value : public Node {
private:
    std::string _name;

public:
    /**
     * @brief Construct a value node with name
     *
     * See \link isValidTag \endlink for valid tag names
     *
     * @param name Variable name to replace
     * @exception Throws templet::exception::InvalidTagError if invalid tag name
     */
    Value(std::string name);

    void evaluate(std::ostream& os, const DataMap& kv) const override;

    NodeType type() const override;
};

/**
 * @brief The IfValue class represents a conditional block in the template
 *
 * Conditional blocks can be omitted/included from the output
 */
class IfValue : public Node {
protected:
    std::string _name;
    std::vector<std::shared_ptr<Node>> _nodes;

public:
    /**
     * @brief Construct an if block node with name
     *
     * See \link isValidTag \endlink for valid tag names
     *
     * @param name Name of the conditional if block
     * @exception Throws templet::exception::InvalidTagError if invalid tag name
     */
    IfValue(std::string name);

    void setChildren(std::vector<std::shared_ptr<Node>> children) override;

    void evaluate(std::ostream& os, const DataMap& kv) const override;

    NodeType type() const override;
};

class ElifValue : public IfValue {
public:
    ElifValue(std::string name);

    virtual NodeType type() const override;
};

class ElseValue : public Node {
private:
    std::vector<std::shared_ptr<Node>> _nodes;

public:
    ElseValue();

    void setChildren(std::vector<std::shared_ptr<Node>> children) override;

    void evaluate(std::ostream& os, const DataMap& kv) const override;

    NodeType type() const override;
};

/**
 * @brief The ForValue class represents a for statement block
 *
 * For statements can be used to traverse lists
 */
class ForValue : public Node {
private:
    std::string _name;
    std::string _alias;
    std::vector<std::shared_ptr<Node>> _nodes;

public:
    ForValue(std::string name, std::string alias);

    void setChildren(std::vector<std::shared_ptr<Node>> children) override;

    void evaluate(std::ostream& os, const DataMap& kv) const override;

    NodeType type() const override;
};

/**
 * @brief Parse a value tag
 *
 * Ex: {$first_name}
 *
 * @param in String to parse
 * @exception templet::exception::InvalidTagError if invalid tag
 * @return Parsed tag as unique pointer to Value node
 */
std::shared_ptr<Node> parse_value_tag(std::string in);

/**
 * @brief Parse an if value tag
 *
 * Ex: {% if is_admin %}
 *
 * @param in String to parse
 * @exception templet::exception::InvalidTagError if invalid tag
 * @return Parsed tag as unique pointer to IfValue node
 */
std::shared_ptr<Node> parse_ifvalue_tag(std::string in);

std::shared_ptr<Node> parse_elifvalue_tag(std::string in);

/**
 * @brief Parse a for value tag
 *
 * Ex: {% for users as user %}{$ user }{% endfor %}
 *
 * @param in String to parse
 * @exception templet::exception::InvalidTagError if invalid tag
 * @return Parsed tag as unique pointer to ForValue node
 */
std::shared_ptr<Node> parse_forvalue_tag(std::string in);

} // namespace nodes
} // namespace templet

#endif // NODES_HPP
