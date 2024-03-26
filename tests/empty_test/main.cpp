#include <string>

namespace flame
{
	std::string_view mn(std::string_view name)
	{
		constexpr auto prefix1 = std::string_view{ "." };
		constexpr auto prefix2 = std::string_view{ "->" };

		auto start = name.find_last_of(prefix1);
		if (start == std::string::npos)
		{
			start = name.find_last_of(prefix2);
			if (start == std::string::npos)
				start = 0;
			else
				start++;
		}
		else
			start++;

		return name.substr(start, (name.size() - start));
	}
}

using namespace flame;

int main(int argc, char** args) 
{
	auto wtf = mn("abc.def");
	return 0;
}

