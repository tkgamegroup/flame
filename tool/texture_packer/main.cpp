#include <flame/foundation/serialize.h>
#include <flame/foundation/bitmap.h>

#include <functional>

using namespace flame;

int main(int argc, char **args)
{
	std::vector<std::wstring> inputs;
	std::wstring output;
	auto border = false;
	for (auto i = 1; i < argc; i++)
	{
		if (args[i] == std::string("-o"))
		{
			i++;
			if (i < argc)
				output = s2w(args[i]);
		}
		else if (args[i] == std::string("-border"))
			border = true;
		else
			inputs.push_back(s2w(args[i]));
	}

	pack_atlas(inputs, output, border);

	return 0;
}
