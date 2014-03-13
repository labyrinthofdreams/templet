#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include "templet.hpp"

int main()
{
    try {
        auto x = templet::make_data({"hello", "world", "foo"});
        templet::DataMap s;
        s["source_path"] = templet::make_data("AAASSSDDD");
        templet::Templet tpl;
        tpl.setTemplateFromFile("E:\\Projects\\cpp\\tmp-builder-thing\\asd.txt");
        auto result = tpl.parse(s);
        std::cout << result << std::endl;
    }
    catch(const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
}

