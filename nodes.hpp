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

#include <memory>
#include <string>
#include <vector>
#include "types.hpp"

namespace templet {
namespace nodes {

using ::templet::types::DataMap;

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
     * Throws if the derived node doesn't support children
     */
    virtual void setChildren(std::vector<std::unique_ptr<Node>>&& /*newNodes*/);

    /**
     * @brief Evaluates the Node and outputs the computed value in ostream os
     */
    virtual void evaluate(std::ostream& /*os*/, DataMap& /*kv*/) const = 0;
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

    void evaluate(std::ostream& os, DataMap& /*kv*/) const override;
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
     * @param name Variable name to replace
     */
    Value(std::string name);

    void evaluate(std::ostream& os, DataMap& kv) const override;
};

/**
 * @brief The IfValue class represents a conditional block in the template
 *
 * Conditional blocks can be omitted/included from the output
 */
class IfValue : public Node {
private:
    std::string _name;
    std::vector<std::unique_ptr<Node>> _nodes;

public:
    /**
     * @brief Construct a conditional if node with name
     * @param name Name of the conditional if block
     */
    IfValue(std::string name);

    void setChildren(std::vector<std::unique_ptr<Node>>&& children) override;

    void evaluate(std::ostream& os, DataMap& kv) const override;
};

} // namespace nodes
} // namespace templet

#endif // NODES_HPP
