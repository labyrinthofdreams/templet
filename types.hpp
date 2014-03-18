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

#ifndef TYPES_HPP
#define TYPES_HPP

#include <initializer_list>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "ptrutil.hpp"

// TODO: Add conversion operators to data classes

namespace templet {
namespace types {

class Data;
using DataPtr = std::unique_ptr<Data>;
using DataVector = std::vector<DataPtr>;
using DataMap = std::map<std::string, DataPtr>;

enum class DataType {
    Invalid,
    String,
    List,
    Mapper
};

/**
 * @brief The base Data class wraps user data in an interface
 * that is compatible with the library
 */
class Data {
public:
    /**
     * @brief Check if data has been set
     * @return True if data has been set, otherwise false
     */
    virtual bool empty() const = 0;

    /**
     * @brief Get string value from object
     *
     * Throws if the derived class doesn't support this type
     *
     * @return Value as a string
     */
    virtual std::string getValue() const;

    /**
     * @brief Get list of values from object
     *
     * Throws if the derived class doesn't support this type
     *
     * @return List of values as a vector
     */
    virtual const DataVector& getList() const;

    /**
     * @brief Get map values from object
     * @exception std::runtime_error if the derived class doesn't support this type
     * @return Map values
     */
    virtual const DataMap& getMap() const;

    virtual DataType type() const;
};

/**
 * @brief The DataValue class wraps a string
 */
class DataValue : public Data {
private:
    std::string _value;

public:
    /**
     * @brief Construct a DataValue with a string
     * @param value String value
     */
    DataValue(std::string value);
    bool empty() const override;
    std::string getValue() const override;
    DataType type() const override;
};

/**
 * @brief The DataList class wraps a vector
 */
class DataList : public Data {
private:
    DataVector _data;

public:
    /**
     * @brief Construct a DataList with a vector of values
     * @param data Vector of values
     */
    DataList(DataVector&& data);

    /**
     * @brief Construct a Datalist with an initializer list of strings
     * @param items Initializer list
     */
    DataList(std::initializer_list<std::string>&& items);

    bool empty() const override;
    const DataVector& getList() const override;
    DataType type() const override;
};

/**
 * @brief The DataMapper class wraps a map
 */
class DataMapper : public Data {
private:
    DataMap _data;

public:
    /**
     * @brief Construct a DataMapper with a map
     * @param data Map
     */
    DataMapper(DataMap&& data);

    // TODO: Add initializer list constructor

    bool empty() const override;
    const DataMap& getMap() const override;
    DataType type() const override;
};

} // namespace types

// Expose user data types in general templet namespace
using types::DataMap;
using types::DataPtr;
using types::DataVector;

/**
 * @brief Wrap a string in a DataPtr
 * @param value String to wrap
 * @return Value wrapped in DataPtr
 */
static types::DataPtr make_data(std::string value) {
    return ::mylib::make_unique<types::DataValue>(std::move(value));
}

/**
 * @brief Wrap a DataVector in a DataPtr
 *
 * Note: This version will move the values from the DataVector
 *
 * @param value Vector to wrap
 * @return Value wrapped in DataPtr
 */
static types::DataPtr make_data(types::DataVector&& value) {
    return ::mylib::make_unique<types::DataList>(std::move(value));
}

/**
 * @brief Wrap a vector of strings in a DataPtr
 * @param value Vector of strings to wrap
 * @return Value wrapped in DataPtr
 */
static types::DataPtr make_data(std::vector<std::string> value) {
    DataVector vec;
    for(auto& v : value) {
        vec.push_back(make_data(std::move(v)));
    }
    return make_data(std::move(vec));
}

/**
 * @brief Wrap an initializer list in a DataPtr
 * @param value Initializer list to wrap
 * @return Value wrapped in DataPtr
 */
static types::DataPtr make_data(std::initializer_list<std::string> value) {
    return ::mylib::make_unique<types::DataList>(std::move(value));
}

static types::DataPtr make_data(types::DataMap&& value) {
    return ::mylib::make_unique<types::DataMapper>(std::move(value));
}

} // namespace templet

#endif // TYPES_HPP
