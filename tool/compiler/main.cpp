#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

using namespace flame;

int main(int argc, char **args)
{
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

	std::vector<std::string> libraries;
	std::ifstream compile_options(L"compile_options.txt");
	while (!compile_options.eof())
	{
		std::string line;
		std::getline(compile_options, line);
		if (!line.empty())
		{
			auto l = std::filesystem::canonical(line).replace_extension(L".lib").string();
			std::replace(l.begin(), l.end(), '\\', '/');
			libraries.push_back(l);
		}
	}
	compile_options.close();

	std::ofstream cmakelists(L"CMakeLists.txt");
	cmakelists << "# THIS FILE IS AUTO GENERATED\n";
	cmakelists << "cmake_minimum_required(VERSION 3.4)\n";
	cmakelists << "project(bp)\n";
	cmakelists << "add_definitions(-W0 -std:c++latest)\n";
	cmakelists << "file(GLOB SOURCE_LIST \"*.c*\")\n";
	cmakelists << "add_library(bp SHARED ${SOURCE_LIST})\n";
	{
		auto path = std::filesystem::canonical(get_app_path().v);
		auto str = (path / L"flame_type.lib").string();
		std::replace(str.begin(), str.end(), '\\', '/');
		cmakelists << "target_link_libraries(bp " << str << ")\n";
	}
	for (auto& l : libraries)
		cmakelists << "target_link_libraries(bp " << l << ")\n";
	cmakelists << "target_include_directories(bp PRIVATE ${CMAKE_SOURCE_DIR}/../../include)\n";
	srand(::time(0));
	auto pdb_filename = std::to_string(::rand() % 100000);
	cmakelists << "set_target_properties(bp PROPERTIES PDB_NAME " << pdb_filename << ")\n";
	cmakelists << "add_custom_command(TARGET bp POST_BUILD COMMAND ${CMAKE_SOURCE_DIR}/../../bin/typeinfogen ${CMAKE_SOURCE_DIR}/build/debug/bp.dll ";
	cmakelists << "-p${CMAKE_SOURCE_DIR}/build/debug/" << pdb_filename << ".pdb)\n";
	cmakelists.close();

	printf(" - done\n");

	printf("cmaking:\n");
	std::wstring cmake_cmd(L"cmake ");
	cmake_cmd += L" -B build";
	exec_and_redirect_to_std_output(nullptr, cmake_cmd.data());

	printf("compiling:\n");
	exec_and_redirect_to_std_output(nullptr, wsfmt(L"%s/Common7/IDE/devenv.com \"%s/build/bp.sln\" /build debug", s2w(VS_LOCATION).c_str(), get_curr_path().v).data());

	system("pause");

	return 0;
}
