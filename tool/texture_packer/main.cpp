#include <flame/serialize.h>
#include <flame/foundation/bitmap.h>

#include <functional>

using namespace flame;

int main(int argc, char **args)
{
	std::vector<std::wstring> _inputs;
	std::vector<const wchar_t*> inputs;
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
			_inputs.push_back(s2w(args[i]));
	}
	for (auto& i : _inputs)
		inputs.push_back(i.c_str());

	pack_atlas(inputs.size(), inputs.data(), output.c_str(), border);

	return 0;
}
