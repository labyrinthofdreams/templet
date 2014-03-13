Templet - Pure C++11 template engine

Currently supports only variables and basic if blocks

Example usage:

templet::DataMap data;
data["first_name"] = templet::make_data("John");
data["last_name"] = templet::make_data("Doe");
templet::Templet tpl("Hello, {$first_name} {$last_name}!");
std::cout << tpl.parse(data); // Outputs: "Hello, John Doe!"

Syntax:

Note: Whitespace before and after the variable name is ignored

Variables:

{$first_name}

To access: Define the value in a DataMap and pass it to Templet::parse

templet::DataMap data;
data["first_name"] = templet::make_data("John");

If blocks:

{% if has_money %}
{% endif %}

To access: Set the value in a DataMap and pass it to Templet::parse

templet::DataMap data;
data["has_money"] = templet::make_data("true");