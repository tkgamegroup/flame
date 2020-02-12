#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

using namespace flame;

int main(int argc, char **args)
{
	std::filesystem::path root;
	std::filesystem::path dst;
	std::vector<std::filesystem::path> items;

	for (auto i = 1; i < argc; i++)
	{
		auto arg = std::string(args[i]);
		if (arg[0] == '-' && arg.size() > 1)
		{
			if (arg[1] == 'r' && arg.size() > 2)
				root = std::string(arg.begin() + 2, arg.end());
			else if (arg[1] == 'd' && arg.size() > 2)
				dst = std::string(arg.begin() + 2, arg.end());
		}
		else
			items.push_back(arg);
	}

	for (auto& i : items)
	{
		auto p = dst / i.parent_path();
		if (!std::filesystem::exists(p))
			std::filesystem::create_directories(p);
		std::filesystem::copy_file(root / i, dst / i, std::filesystem::copy_options::overwrite_existing);
	}

	return 0;
}
