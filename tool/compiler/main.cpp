#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

using namespace flame;

int main(int argc, char **args)
{
	std::string name;
	std::vector<std::string> libraries;

	for (auto i = 1; i < argc; i++)
	{
		auto arg = std::string(args[i]);
		if (arg[0] == '-' && arg.size() > 1)
		{
			if (arg[1] == 'n' && arg.size() > 2)
				name = std::string(arg.begin() + 2, arg.end());
			else if (arg[1] == 'l' && arg.size() > 2)
			{
				auto l = std::filesystem::canonical(std::string(arg.begin() + 2, arg.end())).replace_extension(L".lib").string();
				std::replace(l.begin(), l.end(), '\\', '/');
				libraries.push_back(l);
			}
		}
	}

	auto path = std::filesystem::path(L"build/Debug");
	if (std::filesystem::exists(path))
	{
		std::vector<std::filesystem::path> pdbs;
		for (std::filesystem::directory_iterator end, it(path); it != end; it++)
		{
			if (!std::filesystem::is_directory(it->status()))
			{
				auto ext = it->path().extension().wstring();
				if (ext == L".pdb" && !is_file_occupied(it->path().wstring().c_str()))
					pdbs.push_back(it->path());
			}
		}
		for (auto& p : pdbs)
			std::filesystem::remove(p);
	}

	printf("generating cmakelists");

	std::ofstream cmakelists(L"CMakeLists.txt");
	cmakelists << "# THIS FILE IS AUTO GENERATED\n";
	cmakelists << "cmake_minimum_required(VERSION 3.16.4)\n";
	cmakelists << "project(" << name << ")\n";
	auto flame_path = std::string(getenv("FLAME_PATH"));
	for (auto& ch : flame_path)
	{
		if (ch == '\\')
			ch = '/';
	}
	cmakelists << "include(\"" << flame_path << "/cmake_utils.cmake\")\n";
	cmakelists << "add_definitions(-W0 -std:c++latest)\n";
	cmakelists << "file(GLOB SOURCE_LIST \"*.c*\")\n";
	cmakelists << "generate_rc()\n";
	cmakelists << "add_library(" << name << " SHARED ${SOURCE_LIST} \"${CMAKE_CURRENT_BINARY_DIR}/version.rc\")\n";
	{
		auto path = std::filesystem::canonical(get_app_path().v);
		auto str = (path / L"flame_type.lib").string();
		std::replace(str.begin(), str.end(), '\\', '/');
		cmakelists << "target_link_libraries(" << name << " \"" << str << "\")\n";
	}
	for (auto& l : libraries)
		cmakelists << "target_link_libraries(" << name << " \"" << l << "\")\n";
	cmakelists << "target_include_directories(" << name << " PRIVATE ${CMAKE_SOURCE_DIR}/../../include)\n";
	srand(::time(0));
	auto pdb_filename = std::to_string(::rand() % 100000);
	cmakelists << "set_target_properties(" << name << " PROPERTIES PDB_NAME " << pdb_filename << ")\n";
	cmakelists << "add_custom_command(TARGET " << name << " POST_BUILD COMMAND \"${CMAKE_SOURCE_DIR}/../../bin/typeinfogen\" \"${CMAKE_SOURCE_DIR}/build/debug/" << name << ".dll\" -p" << pdb_filename << ")\n";
	cmakelists.close();

	printf(" - done\n");

	printf("cmaking:\n");
	std::wstring cmake_cmd(L"cmake ");
	cmake_cmd += L" -B build";
	exec_and_redirect_to_std_output(nullptr, cmake_cmd.data());

	printf("compiling:\n");
	exec_and_redirect_to_std_output(nullptr, wfmt(L"%s/Common7/IDE/devenv.com \"%s/build/bp.sln\" /build debug", s2w(VS_LOCATION).c_str(), get_curr_path().v).data());

	system("pause");

	return 0;
}
