#include <flame/serialize.h>
#include <flame/foundation/bitmap.h>

using namespace flame;

int main(int argc, char **args)
{
	std::vector<std::wstring> _inputs;
	std::vector<const wchar_t*> inputs;
	std::wstring output;
	auto border = false;
	for (auto i = 1; i < argc; i++)
	{
		auto arg = std::string(args[i]);
		if (arg[0] == '-' && arg.size() > 1)
		{
			if (arg[1] == 'b')
				border = true;
			else if (arg[1] == 'o' && arg.size() > 2)
				output = s2w(std::string(arg.begin() + 2, arg.end()));
		}
	}
	for (std::filesystem::directory_iterator end, it(get_curr_path().v); it != end; it++)
	{
		if (!std::filesystem::is_directory(it->status()) && is_image_file(it->path().extension()))
			_inputs.push_back(it->path().wstring());
	}
	for (auto& i : _inputs)
		inputs.push_back(i.c_str());

	pack_atlas(inputs.size(), inputs.data(), output.c_str(), border);

	return 0;
}
