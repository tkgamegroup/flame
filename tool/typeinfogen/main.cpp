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

	std::wstring dll_filename = s2w(args[1]);
	std::wstring typeinfo_filename = ext_replace(dll_filename, L".typeinfo");
	std::vector<std::wstring> dependencies;
	for (auto i = 2; i < argc; i++)
		dependencies.push_back(s2w(args[i]));

	if (!std::fs::exists(typeinfo_filename) || std::fs::last_write_time(typeinfo_filename) < std::fs::last_write_time(dll_filename))
	{
		printf("typeinfo generating\n");

		for (auto& d : dependencies)
		{
			if (std::fs::exists(d))
				typeinfo_load(d);
		}

		typeinfo_collect(dll_filename);
		typeinfo_save(typeinfo_filename, dll_filename);
	}
	else
		printf("typeinfo up to data\n");

	return 0;
}
