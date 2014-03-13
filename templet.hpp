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

#ifndef TEMPLET_HPP
#define TEMPLET_HPP

#include <fstream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include "nodes.hpp"
#include "types.hpp"

namespace templet {
namespace helpers {

/**
 * @brief Reads file contents and returns it as a string
 */
struct FileReader {
    /**
     * @brief Read file contents
     * @param path Path to file
     * @exception std::runtime_error Thrown if file can't be opened
     * @return Contents of the file as a string
     */
    static std::string fromFile(const std::string& path) {
        std::ifstream infile {path, std::ios_base::in};
        if(!infile.is_open()) {
            throw std::runtime_error("File not found");
        }

        std::stringstream ss;
        ss << infile.rdbuf();

        return ss.str();
    }
};

} // namespace helpers

/**
 * @brief The Templet class parses templates
 *
 * Example usage:
 *
 * templet::DataMap data;\n
 * data["first_name"] = templet::make_data("John");\n
 * data["last_name"] = templet::make_data("Doe");\n
 * templet::Templet tpl("Hello, {$first_name} {$last_name}!");\n
 * std::cout << tpl.parse(data); // Outputs: "Hello, John Doe!"
 *
 */
class Templet {
private:
    std::string _text;
    std::stringstream _parsed;
    std::vector<std::unique_ptr<nodes::Node>> _nodes;

    /**
     * @brief Reset internal state
     */
    void reset();

    /**
     * @brief Tokenize a string into a vector of nodes
     * @param in String to tokenize
     * @return Vector of tokenized nodes
     */
    std::vector<std::unique_ptr<nodes::Node>> tokenize(std::string &in);

public:
    Templet() = default;

    /**
     * @brief Construct Templet object with template text
     * @param text Template text
     */
    Templet(std::string text);

    /**
     * @brief readFromFile
     *
     * Set template from file
     *
     * Throws if FileReaderT throws
     *
     * @param path Path to file
     */
    template <class FileReaderT = helpers::FileReader>
    void setTemplateFromFile(const std::string& path) {
        auto s = FileReaderT::fromFile(path);
        setTemplate(std::move(s));
    }

    /**
     * @brief setTemplate
     *
     * Set template from string
     *
     * @param str Template string
     */
    void setTemplate(std::string str);

    /**
     * @brief parse
     *
     * Parse the template and return parsed result as a string
     *
     * If the member function has already been called on the same
     * template the old result is returned.
     * Call \link setTemplate \endlink or \link setTemplateFromFile \endlink
     * for the member function to re-parse the template.
     *
     * @param values Map of key-value pairs for parsing the template
     * @return Parsed template as a string
     */
    std::string parse(templet::DataMap& values);

    /**
     * @brief result
     * @return Parsed template as a string
     */
    std::string result() const;
};

} // namespace templet

#endif // TEMPLET_HPP
