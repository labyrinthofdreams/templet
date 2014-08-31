#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
#include "gtest/gtest.h"
#include "ptrutil.hpp"
#include "templet.hpp"

class TempletParserTest : public ::testing::Test {
protected:
    templet::Templet tpl;
    templet::DataMap map;
};

using namespace templet;

TEST(FreeParseFunctionTest, SimpleSubstitution) {
    templet::DataMap map;
    map["name"] = templet::make_data("John");

    std::ostringstream os;
    templet::parse("hello {$name}", map, os);

    EXPECT_EQ(os.str(), "hello John");
}

//
// Test the make_data functions
//

TEST(MakeDataHelperTest, StringToDataPtr) {
    DataPtr res = make_data("john");
    EXPECT_EQ(res->getValue(), "john");
}

TEST(MakeDataHelperTest, StringToDataPtrIsNotEmpty) {
    DataPtr res = make_data("john");
    EXPECT_EQ(res->empty(), false);
}

TEST(MakeDataHelperTest, StringToDataPtrIsEmpty) {
    DataPtr res = make_data("");
    EXPECT_EQ(res->empty(), true);

    std::string empty;
    res = make_data(empty);
    EXPECT_EQ(res->empty(), true);
}

TEST(MakeDataHelperTest, VectorToDataPtr) {
    DataVector xs;
    xs.push_back(make_data("john"));
    xs.push_back(make_data("doe"));

    DataPtr res = make_data(std::move(xs));
    const DataVector& ys = res->getList();

    ASSERT_EQ(xs.size(), 0);
    ASSERT_EQ(ys.size(), 2);
    EXPECT_EQ(ys[0]->getValue(), "john");
    EXPECT_EQ(ys[1]->getValue(), "doe");
}

TEST(MakeDataHelperTest, VectorToDataPtrIsNotEmpty) {
    DataVector xs;
    xs.push_back(make_data("john"));
    xs.push_back(make_data("doe"));

    DataPtr res = make_data(std::move(xs));

    EXPECT_EQ(res->empty(), false);
}

TEST(MakeDataHelperTest, VectorToDataPtrIsEmpty) {
    DataVector xs;

    DataPtr res = make_data(std::move(xs));

    EXPECT_EQ(res->empty(), true);
}

TEST(MakeDataHelperTest, StringInitializerListRValueToDataPtr) {
    DataPtr res = make_data({"first", "second", "third"});

    const DataVector& r = res->getList();
    ASSERT_EQ(r.size(), 3);
    EXPECT_EQ(r[0]->getValue(), "first");
    EXPECT_EQ(r[1]->getValue(), "second");
    EXPECT_EQ(r[2]->getValue(), "third");
}

TEST(MakeDataHelperTest, StringInitializerListLValueToDataPtr) {
    std::initializer_list<std::string> xs {"first", "second", "third"};
    DataPtr res = make_data(xs);

    const DataVector& r = res->getList();
    ASSERT_EQ(r.size(), 3);
    EXPECT_EQ(r[0]->getValue(), "first");
    EXPECT_EQ(r[1]->getValue(), "second");
    EXPECT_EQ(r[2]->getValue(), "third");
}

TEST(MakeDataHelperTest, StringVectorToDataPtr) {
    std::vector<std::string> xs {"first", "second", "third"};

    // std::move not necessary as the parameter is by-value
    DataPtr res = make_data(std::move(xs));

    const DataVector& r = res->getList();
    ASSERT_EQ(r.size(), 3);
    EXPECT_EQ(r[0]->getValue(), "first");
    EXPECT_EQ(r[1]->getValue(), "second");
    EXPECT_EQ(r[2]->getValue(), "third");
}

//
// Test the parser
//

TEST_F(TempletParserTest, EmptyTemplate) {
    tpl.setTemplate("");
    EXPECT_EQ(tpl.parse(map), "");
}

TEST_F(TempletParserTest, PlainText) {
    tpl.setTemplate("hello world");
    ASSERT_EQ(tpl.parse(map), "hello world");
}

TEST_F(TempletParserTest, Result) {
    tpl.setTemplate("hello world");
    EXPECT_EQ(tpl.parse(map), "hello world");
    EXPECT_EQ(tpl.result(), "hello world");
    EXPECT_EQ(tpl.result(), "hello world");

    tpl.setTemplate("foo bar baz");
    EXPECT_EQ(tpl.result(), "");
    EXPECT_EQ(tpl.parse(map), "foo bar baz");
    EXPECT_EQ(tpl.result(), "foo bar baz");
    EXPECT_EQ(tpl.result(), "foo bar baz");
}

TEST_F(TempletParserTest, ResultAfterInstantiation) {
    EXPECT_EQ(tpl.result(), "");
}

TEST_F(TempletParserTest, ResultAfterInvalidFilePath) {
    ASSERT_ANY_THROW(tpl.setTemplateFromFile("badfile.tpl"));
    EXPECT_EQ(tpl.result(), "");
}

TEST_F(TempletParserTest, ResultAfterInvalidFile) {
    // Parse one template
    tpl.setTemplate("hello world");
    tpl.parse(map);

    // Try to load a bad file
    ASSERT_ANY_THROW(tpl.setTemplateFromFile("badfile.tpl"));

    // Should still return the previous parsed template
    EXPECT_EQ(tpl.result(), "hello world");
}

TEST_F(TempletParserTest, UnrecognizedTag) {
    tpl.setTemplate("hello {world}");
    EXPECT_EQ(tpl.parse(map), "hello {world}");

    tpl.setTemplate("hello {*world}");
    EXPECT_EQ(tpl.parse(map), "hello {*world}");

    tpl.setTemplate("hello {% infloop %}world{% endinfloop %}");
    ASSERT_ANY_THROW(tpl.parse(map));
}

TEST_F(TempletParserTest, IgnoredTag) {
    tpl.setTemplate("hello {\\world}");
    EXPECT_EQ(tpl.parse(map), "hello {world}");

    tpl.setTemplate("hello {\\\\world}");
    EXPECT_EQ(tpl.parse(map), "hello {\\world}");

    tpl.setTemplate("hello {\\\\\\world}");
    EXPECT_EQ(tpl.parse(map), "hello {\\\\world}");

    tpl.setTemplate("hello {\\*world}");
    EXPECT_EQ(tpl.parse(map), "hello {*world}");

    tpl.setTemplate("hello {\\$world}");
    EXPECT_EQ(tpl.parse(map), "hello {$world}");
}

TEST_F(TempletParserTest, EndsWithTagOpener) {
    tpl.setTemplate("hello world {");
    EXPECT_EQ(tpl.parse(map), "hello world {");

    tpl.setTemplate("hello world {$");
    EXPECT_EQ(tpl.parse(map), "hello world {$");

    tpl.setTemplate("hello world {%");
    EXPECT_EQ(tpl.parse(map), "hello world {%");
}

TEST_F(TempletParserTest, EndsWithIncompleteTag) {
    tpl.setTemplate("hello world { foo");
    EXPECT_EQ(tpl.parse(map), "hello world { foo");

    tpl.setTemplate("hello world {$ foo");
    EXPECT_EQ(tpl.parse(map), "hello world {$ foo");

    tpl.setTemplate("hello world {% foo");
    EXPECT_EQ(tpl.parse(map), "hello world {% foo");
}

TEST_F(TempletParserTest, UnsetVariables) {
    tpl.setTemplate("hello, {$first_name} {$last_name}");
    ASSERT_EQ(tpl.parse(map), "hello,  ");
}

TEST_F(TempletParserTest, SetVariables) {
    tpl.setTemplate("hello, {$first_name} {$last_name}");
    map["first_name"] = make_data("john");
    map["last_name"] = make_data("doe");
    ASSERT_EQ(tpl.parse(map), "hello, john doe");
}

TEST_F(TempletParserTest, ParseWithDifferentValues) {
    tpl.setTemplate("hello, {$first_name} {$last_name}");

    map["first_name"] = make_data("john");
    map["last_name"] = make_data("doe");
    EXPECT_EQ(tpl.parse(map), "hello, john doe");

    map["first_name"] = make_data("jane");
    map["last_name"] = make_data("roe");
    EXPECT_EQ(tpl.parse(map), "hello, jane roe");
}

TEST_F(TempletParserTest, InvalidValueTagName) {
    tpl.setTemplate("{$foo&bar}");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);

    tpl.setTemplate("{$foo bar}");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);

    // Special case where the string must be instantiated with
    // the string constructor that accepts the length
    tpl.setTemplate(std::string("{$foo\0&bar}", 11));
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);
}

TEST_F(TempletParserTest, ValidValueTagName) {
    tpl.setTemplate("{$azAZ09-_}");
    ASSERT_NO_THROW(tpl.parse(map));
}

TEST_F(TempletParserTest, ValueTagNameWithOuterSpace) {
    tpl.setTemplate("{$   azAZ09-_   }");
    ASSERT_NO_THROW(tpl.parse(map));
}

//
// Test 'If' statement blocks
//

TEST_F(TempletParserTest, InvalidIfValueTagName) {
    tpl.setTemplate("{% if foo&bar %}");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);

    tpl.setTemplate("{% if foo bar %}");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);
}

TEST_F(TempletParserTest, ValidIfValueTagName) {
    tpl.setTemplate("{% if azAZ09_- %}");
    ASSERT_NO_THROW(tpl.parse(map));
}

TEST_F(TempletParserTest, IfValueTagNameWithOuterSpace) {
    // TODO: Space should not be allowed between if and name
    tpl.setTemplate("{%    if    azAZ09_-    %}");
    ASSERT_NO_THROW(tpl.parse(map));
}

TEST_F(TempletParserTest, InvalidFilePath) {
    ASSERT_ANY_THROW(tpl.setTemplateFromFile("badfile.tpl"));
}

TEST_F(TempletParserTest, ParseValidFile) {
    ASSERT_NO_THROW(tpl.setTemplateFromFile("example.tpl"));
    map["first_name"] = make_data("john");
    map["last_name"] = make_data("doe");
    EXPECT_EQ(tpl.parse(map), "Hello, john doe");
}

TEST_F(TempletParserTest, UnsetIfBlock) {
    tpl.setTemplate("This is {% if is_not_test %}not {% endif %}a test");
    EXPECT_EQ(tpl.parse(map), "This is a test");
}

TEST_F(TempletParserTest, SetIfBlock) {
    tpl.setTemplate("This is {% if is_not_test %}not {% endif %}a test");
    map["is_not_test"] = make_data("true");
    EXPECT_EQ(tpl.parse(map), "This is not a test");
}

TEST_F(TempletParserTest, UnsetUnclosedIfBlock) {
    tpl.setTemplate("Hello {% if is_world %}world");
    EXPECT_EQ(tpl.parse(map), "Hello ");
}

TEST_F(TempletParserTest, SetUnclosedIfBlock) {
    tpl.setTemplate("Hello {% if is_world %}world");
    map["is_world"] = make_data("true");
    EXPECT_EQ(tpl.parse(map), "Hello world");
}

TEST_F(TempletParserTest, IfBlockTextAfterEndif) {
    tpl.setTemplate("Hello{% if is_world %} world{% endif %}. End of file.");
    EXPECT_EQ(tpl.parse(map), "Hello. End of file.");

    map["is_world"] = make_data("true");
    EXPECT_EQ(tpl.parse(map), "Hello world. End of file.");
}

TEST_F(TempletParserTest, IfBlockNestedDupeName) {
    tpl.setTemplate("{% if is_world %}{% if is_world %}Hello{% endif %}{% endif %}");
    map["is_world"] = make_data("true");
    EXPECT_EQ(tpl.parse(map), "Hello");
}

//
// Test If-Else statement blocks
//

TEST_F(TempletParserTest, IfElseBlock) {
    tpl.setTemplate("{% if debug %}Debug mode{% else %}Release mode{% endif %}");
    EXPECT_EQ(tpl.parse(map), "Release mode");

    map["debug"] = make_data("true");
    EXPECT_EQ(tpl.parse(map), "Debug mode");
}

TEST_F(TempletParserTest, IfElseBlockMultipleElses) {
    tpl.setTemplate("{% if debug %}Debug mode{% else %}Release mode{% else %}, not debug{% endif %}");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);
}

//
// Test Elif statement blocks
//

TEST_F(TempletParserTest, ElifBlock) {
    tpl.setTemplate("{% if debug %}Debug mode{% elif test %}Test mode{% else %}Release mode{% endif %}");
    EXPECT_EQ(tpl.parse(map), "Release mode");

    map["test"] = make_data("true");
    EXPECT_EQ(tpl.parse(map), "Test mode");

    map.erase("test");
    map["debug"] = make_data("true");
    EXPECT_EQ(tpl.parse(map), "Debug mode");
}

TEST_F(TempletParserTest, ElifBlockMultiple) {
    tpl.setTemplate("{% if debug %}Debug mode{% elif test %}Test mode{% elif gravity %}Gravity mode{% else %}Release mode{% endif %}");
    EXPECT_EQ(tpl.parse(map), "Release mode");

    map["test"] = make_data("true");
    EXPECT_EQ(tpl.parse(map), "Test mode");

    map.erase("test");
    map["gravity"] = make_data("true");
    EXPECT_EQ(tpl.parse(map), "Gravity mode");

    map.erase("test");
    map["debug"] = make_data("true");
    EXPECT_EQ(tpl.parse(map), "Debug mode");
}

TEST_F(TempletParserTest, IfInsideIf) {
    tpl.setTemplate("{% if debug %}Debug mode{% if test %}Test mode{% endif %}");

    map["debug"] = make_data("true");
    EXPECT_EQ(tpl.parse(map), "Debug mode");

    map["test"] = make_data("true");
    EXPECT_EQ(tpl.parse(map), "Debug modeTest mode");

    map.erase("debug");
    EXPECT_EQ(tpl.parse(map), "");
}

TEST_F(TempletParserTest, IfInsideElif) {
    tpl.setTemplate("{% if debug %}Debug mode{% elif test %}Test mode{% if gravity %}Gravity{% endif %}{% endif %}");

    map["test"] = make_data("true");
    map["gravity"] = make_data("true");
    EXPECT_EQ(tpl.parse(map), "Test modeGravity");
}

TEST_F(TempletParserTest, IfInsideElse) {
    tpl.setTemplate("{% if debug %}Debug mode{% elif test %}Test mode{% else %}Release mode{% if gravity %}Gravity{% endif %}{% endif %}");

    map["gravity"] = make_data("true");
    EXPECT_EQ(tpl.parse(map), "Release modeGravity");
}

TEST_F(TempletParserTest, IfDotNotation) {
    DataMap data;
    data["hostname"] = make_data("localhost");

    DataMap server;
    server["server"] = make_data(data);

    map["config"] = make_data(server);
    tpl.setTemplate("{% if config.server.hostname %}{$ config.server.hostname }{% endif %}");
    EXPECT_EQ(tpl.parse(map), "localhost");

    tpl.setTemplate("{% if config.server.ip %}{$ config.server.ip }{% endif %}");
    EXPECT_EQ(tpl.parse(map), "");
}

TEST_F(TempletParserTest, IfDotNotationWithArrays) {
    DataMap data;
    data["hostname"] = make_data("localhost");

    DataVector server_data;
    server_data.push_back(make_data(data));

    DataMap servers;
    servers["servers"] = make_data(server_data);

    map["config"] = make_data(servers);
    tpl.setTemplate("{% if config.servers[0].hostname %}{$ config.servers[0].hostname }{% endif %}");
    EXPECT_EQ(tpl.parse(map), "localhost");

    tpl.setTemplate("{% if config.servers[1].hostname %}{$ config.servers[1].hostname }{% endif %}");
    EXPECT_EQ(tpl.parse(map), "");
}

TEST_F(TempletParserTest, IfDotNotationWithArraysEndIndex) {
    DataMap data;
    data["hostnames"] = make_data({"localhost", "game-server"});

    DataVector server_data;
    server_data.push_back(make_data(data));

    DataMap servers;
    servers["servers"] = make_data(server_data);

    map["config"] = make_data(servers);
    tpl.setTemplate("{% if config.servers[0].hostnames[0] %}{$ config.servers[0].hostnames[0] }{% endif %}");
    EXPECT_EQ(tpl.parse(map), "localhost");

    tpl.setTemplate("{% if config.servers[0].hostnames[2] %}{$ config.servers[0].hostnames[2] }{% endif %}");
    EXPECT_EQ(tpl.parse(map), "");
}

//
//  Test Elif-Else statements without preceding If/Elif statements
//

TEST_F(TempletParserTest, ElifWithoutIf) {
    tpl.setTemplate("{% elif debug %}Debug mode{% endif %}");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);
}

TEST_F(TempletParserTest, ElseWithoutIfOrElif) {
    tpl.setTemplate("{% else %}Debug mode{% endif %}");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);
}

//
// Test array index operator
//

TEST_F(TempletParserTest, ArrayAccess) {
    tpl.setTemplate("Items in a list: {$ items[0] }, {$ items[1] }, {$ items[2] }");
    map["items"] = make_data({"first", "second", "third"});
    EXPECT_EQ(tpl.parse(map), "Items in a list: first, second, third");
}

TEST_F(TempletParserTest, ArrayAccessIgnoreLeadingZero) {
    tpl.setTemplate("Items in a list: {$ items[00] }, {$ items[01] }, {$ items[02] }");
    map["items"] = make_data({"first", "second", "third"});
    EXPECT_EQ(tpl.parse(map), "Items in a list: first, second, third");
}

TEST_F(TempletParserTest, ArrayAccessOutOfRange) {
    tpl.setTemplate("Items in a list: {$ items[3] }");
    map["items"] = make_data({"first", "second", "third"});
    EXPECT_EQ(tpl.parse(map), "Items in a list: ");
}

TEST_F(TempletParserTest, ArrayAccessOutOfRangeNegative) {
    tpl.setTemplate("Items in a list: {$ items[-1] }");
    map["items"] = make_data({"first", "second", "third"});
    EXPECT_EQ(tpl.parse(map), "Items in a list: ");
}

TEST_F(TempletParserTest, ArrayAccessInvalidNumbers) {
    map["items"] = make_data({"first", "second", "third"});

    tpl.setTemplate("Value: {$ items[1.56] }");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);

    tpl.setTemplate("Value: {$ items[0x01] }");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);
}

TEST_F(TempletParserTest, ArrayAccessInvalidFormat) {
    map["items"] = make_data({"first", "second", "third"});

    tpl.setTemplate("Items in a list: {$ items[[0]] }");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);

    tpl.setTemplate("Items in a list: {$ items[0 }");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);

    tpl.setTemplate("Items in a list: {$ items[x] }");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);

    tpl.setTemplate("Items in a list: {$ items[] }");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);
}

TEST_F(TempletParserTest, ArrayAccessString) {
    map["item"] = make_data("hello world");
    map["items"] = make_data({"first", "second", "third"});

    tpl.setTemplate("{$ items[0][0] }");
    EXPECT_EQ(tpl.parse(map), "");

    tpl.setTemplate("{$ item[0] }");
    EXPECT_EQ(tpl.parse(map), "");
}

TEST_F(TempletParserTest, ListsOfLists) {
    DataPtr a = make_data({"one", "two", "three"});
    DataPtr b = make_data({"four", "five", "six"});
    DataVector ab;
    ab.push_back(a);
    ab.push_back(b);
    map["items"] = make_data(std::move(ab));

    tpl.setTemplate("{$ items[0][1] } {$ items[1][1] }");
    EXPECT_EQ(tpl.parse(map), "two five");
}


/*TEST_F(TempletParserTest, ArrayAccessStrings) {
    tpl.setTemplate("String is: {$ item[0] }{$ item[1] }{$ item[2] }");
    map["item"] = make_data("foo");
    EXPECT_EQ(tpl.parse(map), "String is: foo");
}

TEST_F(TempletParserTest, ArrayAccessStringsOutOfRange) {
    tpl.setTemplate("String is: {$ item[3] }");
    map["item"] = make_data("foo");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);
}

TEST_F(TempletParserTest, ArrayAccessStringsOutOfRangeNegative) {
    tpl.setTemplate("String is: {$ item[-1] }");
    map["item"] = make_data("foo");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);
}

TEST_F(TempletParserTest, ArrayAccessStringsIgnoreLeadingZero) {
    tpl.setTemplate("String is: {$ item[00] }{$ item[01] }{$ item[02] }");
    map["item"] = make_data("foo");
    EXPECT_EQ(tpl.parse(map), "String is: foo");
}

TEST_F(TempletParserTest, ArrayAccessStringsInvalidFormat) {
    map["item"] = make_data("foo");

    tpl.setTemplate("Item is: {$ item[[0]] }");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);

    tpl.setTemplate("Item is: {$ item[0][0] }");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);

    tpl.setTemplate("Item is: {$ item[0 }");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);

    tpl.setTemplate("Item is: {$ item[x] }");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);

    tpl.setTemplate("Item is: {$ item[] }");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);

    tpl.setTemplate("Item is: {$ item0] }");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);
}*/

TEST_F(TempletParserTest, InvalidDotNotationValue) {
    DataMap config;
    config["hostname"] = make_data("localhost");

    map["config"] = make_data(std::move(config));

    tpl.setTemplate("config.hostname is: {$ config..hostname }");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);

    tpl.setTemplate("config.hostname is: {$ config...hostname }");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);
}

TEST_F(TempletParserTest, DotNotationValue) {
    DataMap config;
    config["hostname"] = make_data("localhost");

    map["config"] = make_data(std::move(config));

    tpl.setTemplate("config.hostname is: {$ config.hostname }");
    EXPECT_EQ(tpl.parse(map), "config.hostname is: localhost");
}

TEST_F(TempletParserTest, DotNotationValueArrayList) {
    DataMap config;
    config["ips"] = make_data({"192.168.101.1", "192.168.101.2", "192.168.101.3"});

    map["server"] = make_data(std::move(config));

    tpl.setTemplate("server.ips[1] is: {$ server.ips[1] }");
    EXPECT_EQ(tpl.parse(map), "server.ips[1] is: 192.168.101.2");
}

TEST_F(TempletParserTest, DotNotationValueArrayListWithoutName) {
    DataMap config;
    config["ips"] = make_data({"192.168.101.1", "192.168.101.2", "192.168.101.3"});

    map["server"] = make_data(std::move(config));

    tpl.setTemplate("server.ips[1] is: {$ server.[1] }");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);

    tpl.setTemplate("server.ips[1] is: {$ .server.ips[1] }");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);
}

TEST_F(TempletParserTest, DotNotationValueArrayMap) {
    DataMap data;
    data["ips"] = make_data({"192.168.101.1", "192.168.101.2", "192.168.101.3"});

    DataMap server;
    server["server"] = make_data(std::move(data));

    map["config"] = make_data(std::move(server));

    tpl.setTemplate("config.server is: {$ config.server }");

    // Can't print maps
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);
}

TEST_F(TempletParserTest, DotNotationValueMulti) {
    DataMap data;
    data["ip"] = make_data("192.168.101.1");

    DataMap server;
    server["server"] = make_data(std::move(data));

    map["config"] = make_data(std::move(server));

    tpl.setTemplate("config.server.ip is: {$ config.server.ip }");
    EXPECT_EQ(tpl.parse(map), "config.server.ip is: 192.168.101.1");
}

TEST_F(TempletParserTest, DotNotationValueMultiArray) {
    DataMap data;
    data["ips"] = make_data({"192.168.101.1", "192.168.101.2", "192.168.101.3"});
    data["hostname"] = make_data("game-server.localhost");

    DataMap data2;
    data2["ips"] = make_data({"192.168.101.100", "192.168.101.101", "192.168.101.102"});
    data2["hostname"] = make_data("stream-server.localhost");

    DataVector servers;
    servers.push_back(make_data(std::move(data)));
    servers.push_back(make_data(std::move(data2)));

    DataMap server;
    server["servers"] = make_data(std::move(servers));

    map["config"] = make_data(std::move(server));

    tpl.setTemplate("config.servers[1].ips[1] is: {$ config.servers[1].ips[1] }");
    EXPECT_EQ(tpl.parse(map), "config.servers[1].ips[1] is: 192.168.101.101");

    tpl.setTemplate("config.servers[1].hostname is: {$ config.servers[1].hostname }");
    EXPECT_EQ(tpl.parse(map), "config.servers[1].hostname is: stream-server.localhost");

    tpl.setTemplate("config.server[1].hostname[1] is: {$ config.server[1].hostname[1] }");
    EXPECT_EQ(tpl.parse(map), "config.server[1].hostname[1] is: ");

    tpl.setTemplate("config.servers[1].hostname[1] is: {$ config.servers[1].hostname[1] }");
    EXPECT_EQ(tpl.parse(map), "config.servers[1].hostname[1] is: ");

    tpl.setTemplate("config.servers.hostname[1] is: {$ config.servers.hostname[1] }");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);
}

TEST_F(TempletParserTest, DotNotationWithoutDots) {
    DataMap data;
    data["ips"] = make_data({"192.168.101.1", "192.168.101.2", "192.168.101.3"});
    data["hostname"] = make_data("game-server.localhost");

    DataVector servers;
    servers.push_back(make_data(std::move(data)));

    DataMap server;
    server["servers"] = make_data(std::move(servers));

    map["config"] = make_data(std::move(server));

    tpl.setTemplate("config.servers[0]ips[1] is: {$ config.servers[0]ips[1] }");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);
}

//
// Tests for 'For' loops
//
// 'For' loops can be used to traverse lists
//

TEST_F(TempletParserTest, ForLoopInvalidSyntax) {
    map["users"] = make_data({"John", "Jane", "Mark", "Mary"});

    tpl.setTemplate("Users: {% for %}{$ user },{% endfor %}");
    ASSERT_THROW(tpl.parse(map), templet::exception::ExpressionSyntaxError);

    tpl.setTemplate("Users: {% for users %}{$ user },{% endfor %}");
    ASSERT_THROW(tpl.parse(map), templet::exception::ExpressionSyntaxError);

    tpl.setTemplate("Users: {% for users as %}{$ user },{% endfor %}");
    ASSERT_THROW(tpl.parse(map), templet::exception::ExpressionSyntaxError);

    tpl.setTemplate("Users: {% for users user %}{$ user },{% endfor %}");
    ASSERT_THROW(tpl.parse(map), templet::exception::ExpressionSyntaxError);

    tpl.setTemplate("Users: {% for users into user %}{$ user },{% endfor %}");
    ASSERT_THROW(tpl.parse(map), templet::exception::ExpressionSyntaxError);
}

TEST_F(TempletParserTest, ForLoopAliasNameCollision) {
    map["users"] = make_data({"John", "Jane", "Mark", "Mary"});
    map["user"] = make_data("root");

    // for expression's user collides with global user
    tpl.setTemplate("Users: {% for users as user %}{$ user }{% endfor %}");
    ASSERT_THROW(tpl.parse(map), templet::exception::InvalidTagError);
}

TEST_F(TempletParserTest, ForLoopInvalidAliasName) {
    // for expression's alias cannot contain . or []
    tpl.setTemplate("{% for servers as user.id %}{% endfor %}");
    ASSERT_ANY_THROW(tpl.parse(map));

    tpl.setTemplate("{% for servers as user[0] %}{% endfor %}");
    ASSERT_ANY_THROW(tpl.parse(map));
}

TEST_F(TempletParserTest, ForLoopList) {
    map["users"] = make_data({"John", "Jane", "Mark", "Mary"});

    tpl.setTemplate("Users: {% for users as user %}{$ user },{% endfor %}");
    EXPECT_EQ(tpl.parse(map), "Users: John,Jane,Mark,Mary,");
}

TEST_F(TempletParserTest, ForLoopListDotNotation) {
    DataMap users;
    users["active"] = make_data({"John", "Jane"});
    users["inactive"] = make_data({"Mark", "Mary"});

    map["users"] = make_data(std::move(users));

    tpl.setTemplate("Users: {% for users.active as user %}{$ user },{% endfor %}");
    EXPECT_EQ(tpl.parse(map), "Users: John,Jane,");
}

TEST_F(TempletParserTest, ForLoopListArrayIndex) {
    DataVector users;
    users.push_back(make_data({"John", "Jane"}));
    users.push_back(make_data({"Mark", "Mary"}));

    map["users"] = make_data(std::move(users));

    tpl.setTemplate("Users: {% for users[0] as user %}{$ user },{% endfor %}");
    EXPECT_EQ(tpl.parse(map), "Users: John,Jane,");
}

TEST_F(TempletParserTest, ForLoopListArrayMultiIndex) {
    DataVector users;
    users.push_back(make_data({"John", "Jane"}));
    users.push_back(make_data({"Mark", "Mary"}));

    DataVector groups;
    groups.push_back(make_data(users));

    map["groups"] = make_data(std::move(groups));

    tpl.setTemplate("Users: {% for groups[0][1] as user %}{$ user },{% endfor %}");
    EXPECT_EQ(tpl.parse(map), "Users: Mark,Mary,");

    tpl.setTemplate("Users: {% for groups[0] as user %}{$ user[0] },{% endfor %}");
    EXPECT_EQ(tpl.parse(map), "Users: John,Mark,");
}

TEST_F(TempletParserTest, ForLoopInnerForLoop) {
    DataVector users;
    users.push_back(make_data({"John", "Jane"}));
    users.push_back(make_data({"Mark", "Mary"}));

    map["users"] = make_data(std::move(users));

    tpl.setTemplate("Users: {% for users as _users %}{% for _users as user %}{$ user },{% endfor %}{% endfor %}");
    EXPECT_EQ(tpl.parse(map), "Users: John,Jane,Mark,Mary,");
}

TEST_F(TempletParserTest, ForLoopMap) {
    DataMap server1;
    server1["name"] = make_data("stream-server");
    server1["ip"] = make_data("192.168.101.1");

    DataMap server2;
    server2["name"] = make_data("game-server");
    server2["ip"] = make_data("192.168.101.100");

    DataVector servers;
    servers.push_back(make_data(std::move(server1)));
    servers.push_back(make_data(std::move(server2)));

    map["servers"] = make_data(std::move(servers));

    tpl.setTemplate("{% for servers as server %}{$ server.ip },{$ server.name }<br>{% endfor %}");
    EXPECT_EQ(tpl.parse(map), "192.168.101.1,stream-server<br>192.168.101.100,game-server<br>");
}

TEST_F(TempletParserTest, ForLoopListOfMapsOfLists) {
    DataMap server1;
    server1["users"] = make_data({"John", "Jane"});

    DataMap server2;
    server2["users"] = make_data({"Mark", "Mary"});

    DataVector servers;
    servers.push_back(make_data(std::move(server1)));
    servers.push_back(make_data(std::move(server2)));

    map["servers"] = make_data(std::move(servers));

    tpl.setTemplate("{% for servers as server %}{% for server.users as user %}{$ user },{% endfor %}{% endfor %}");
    EXPECT_EQ(tpl.parse(map), "John,Jane,Mark,Mary,");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
