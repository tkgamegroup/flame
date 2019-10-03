#include <flame/foundation/foundation.h>
#include <flame/foundation/serialize.h>

using namespace flame;

int main(int argc, char **args)
{
	if (argc < 2)
	{
		printf("argc is less than 2, exit\n");
		return 0;
	}

	std::wstring filename = s2w(args[1]);
	std::wstring typeinfo_filename = ext_replace(filename, L".typeinfo");
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
			dbs.push_back(TypeinfoDatabase::load(dbs, ext_replace(d, L".typeinfo")));

		auto db = TypeinfoDatabase::collect(dbs, filename, pdb_filename);
		TypeinfoDatabase::save(dbs, db);

		printf(" - done\n");
	}
	else
		printf("typeinfo up to data\n");

	return 0;
}
