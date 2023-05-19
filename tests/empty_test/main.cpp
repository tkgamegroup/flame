#include <flame/foundation/foundation.h>
#include <flame/foundation/typeinfo.h>

#include <exprtk.hpp>
#include <iostream>

using namespace flame;

static std::string x = "def";

struct string_assign_t : public exprtk::igeneric_function<float>
{
    using exprtk::igeneric_function<float>::operator();

    inline float operator() (igeneric_function<float>::parameter_list_t parameters)
    {
        if (parameters.size() == 2)
        {
            generic_type& arg0 = parameters[0];
            generic_type& arg1 = parameters[1];
            if (arg0.type == generic_type::e_string && arg1.type == generic_type::e_string)
            {
                auto addr_str = exprtk::to_str(generic_type::string_view(arg0));
                auto str_value = exprtk::to_str(generic_type::string_view(arg1));
                *(std::string*)(s2t<uint64>(addr_str)) = str_value;
            }
        }

        return float(0);
    }
};

int main(int argc, char** args) 
{
	std::string expression_str = "string_assign(return_value, 'abc')";

	exprtk::symbol_table<float> symbols;
	exprtk::expression<float> expression;
	expression.register_symbol_table(symbols);

    string_assign_t string_assign;
    auto addr_str = str((uint64)&x);
	symbols.create_stringvar("return_value", addr_str);
	symbols.add_function("string_assign", string_assign);

	exprtk::parser<float> parser;
	parser.compile(expression_str, expression);

	auto wtf = expression.value();
	printf("%f\n", wtf);

	return 0;
}

