#include <flame/serialize.h>

using namespace flame;

int main(int argc, char **args)
{
	if (argc < 2)
	{
		printf("argc is less than 2, exit\n");
		return 0;
	}

	std::filesystem::path filename(args[1]);
	std::filesystem::path typeinfo_filename = std::filesystem::path(filename).replace_extension(L".typeinfo");
	std::vector<std::wstring> modules;
	std::wstring pdb_filename;
	for (auto i = 2; i < argc; i++)
	{
		auto arg = args[i];
		if (arg[0] == '-')
		{
			switch (arg[1])
			{
			case 'm':
				modules.push_back(s2w(arg + 2));
				break;
			case 'p':
				pdb_filename = s2w(arg + 2);
				break;
			}
		}
	}

	if (!std::filesystem::exists(typeinfo_filename) || std::filesystem::last_write_time(typeinfo_filename) < std::filesystem::last_write_time(filename))
	{
		printf("generating typeinfo");

		std::vector<TypeinfoDatabase*> dbs;
		for (auto& d : modules)
			dbs.push_back(TypeinfoDatabase::load(dbs.size(), dbs.data(), std::filesystem::path(d).replace_extension(L".typeinfo").c_str()));

		auto db = TypeinfoDatabase::collect(dbs.size(), dbs.data(), filename.c_str(), !pdb_filename.empty() ? pdb_filename.c_str() : nullptr);
		TypeinfoDatabase::save(dbs.size(), dbs.data(), db);

		printf(" - done\n");
	}
	else
		printf("typeinfo up to data\n");

	return 0;
}
