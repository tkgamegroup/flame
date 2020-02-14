#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

using namespace flame;

int main(int argc, char **args)
{
	std::filesystem::path root_path;
	std::filesystem::path dst_path;
	std::vector<std::filesystem::path> items;

	for (auto i = 1; i < argc; i++)
	{
		auto arg = std::string(args[i]);
		if (arg[0] == '-' && arg.size() > 1)
		{
			if (arg[1] == 'r' && arg.size() > 2)
				root_path = std::string(arg.begin() + 2, arg.end());
			else if (arg[1] == 'd' && arg.size() > 2)
				dst_path = std::string(arg.begin() + 2, arg.end());
		}
		else
			items.push_back(arg);
	}

	for (auto& i : items)
	{
		auto s = root_path / i;
		auto d = dst_path / i;

		auto dp = d.parent_path();
		if (!std::filesystem::exists(dp))
			std::filesystem::create_directories(dp);

		std::filesystem::copy_file(s, d, std::filesystem::copy_options::overwrite_existing);

		auto ext = s.extension();

		if (ext == L".exe")
		{
			auto dependencies = get_module_dependencies(s.c_str());
			for (auto i = 0; i < dependencies.s; i++)
			{
				auto dep = dependencies[i].str();
				auto ss = s.parent_path() / dep;
				if (std::filesystem::exists(ss))
				{
					auto dd = d.parent_path() / dep;
					std::filesystem::copy_file(ss, dd, std::filesystem::copy_options::overwrite_existing);
					ss.replace_extension(L".typeinfo");
					if (std::filesystem::exists(ss))
					{
						dd.replace_extension(L".typeinfo");
						std::filesystem::copy_file(ss, dd, std::filesystem::copy_options::overwrite_existing);
					}
				}
			}
		}

		if (ext == L".dll" || ext == L".exe")
		{
			s.replace_extension(L".typeinfo");
			if (std::filesystem::exists(s))
			{
				d.replace_extension(L".typeinfo");
				std::filesystem::copy_file(s, d, std::filesystem::copy_options::overwrite_existing);
			}
		}
	}

	return 0;
}
