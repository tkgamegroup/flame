#include <flame/serialize.h>
#include <flame/foundation/bitmap.h>

using namespace flame;

int main(int argc, char **args)
{
	std::vector<std::wstring> _inputs;
	std::vector<const wchar_t*> inputs;
	std::wstring output;
	auto border = false;
	if (argc == 1)
	{
		std::ifstream atlas_options(L"atlas_options.txt");
		while (!atlas_options.eof())
		{
			std::string line;
			std::getline(atlas_options, line);
			if (!line.empty())
			{
				auto sp = ssplit(line);
				if (sp[0] == "border")
					border = true;
				else if (sp[0] == "out")
					output = s2w(sp[1]);
			}
		}
		atlas_options.close();
		for (std::filesystem::directory_iterator end, it(get_curr_path().v); it != end; it++)
		{
			if (!std::filesystem::is_directory(it->status()) && is_image_file(it->path().extension()))
				_inputs.push_back(it->path().wstring());
		}
	}
	else
	{
		for (auto i = 1; i < argc; i++)
		{
			if (args[i] == std::string("-o"))
			{
				i++;
				if (i < argc)
					output = s2w(args[i]);
			}
			else if (args[i] == std::string("-b"))
				border = true;
			else
				_inputs.push_back(s2w(args[i]));
		}
	}
	for (auto& i : _inputs)
		inputs.push_back(i.c_str());

	pack_atlas(inputs.size(), inputs.data(), output.c_str(), border);

	return 0;
}
