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

	auto flame_path = std::string(getenv("FLAME_PATH"));

	std::string pugixml_path;
	std::ifstream cmake_caches(flame_path + "build/CMakeCache.txt");
	while (!cmake_caches.eof())
	{
		std::string line;
		std::getline(cmake_caches, line);
		static std::regex reg_pugixml_path(R"(PUGIXML_PATH:PATH\=(.*))");
		std::smatch res;
		if (std::regex_search(line, res, reg_pugixml_path))
		{
			pugixml_path = res[1].str();
			break;
		}
	}
	cmake_caches.close();

	std::ofstream cmakelists(L"CMakeLists.txt");
	cmakelists << "# THIS FILE IS AUTO GENERATED\n";
	cmakelists << "cmake_minimum_required(VERSION 3.16.4)\n";
	cmakelists << "project(" << name << ")\n";
	for (auto& ch : flame_path)
	{
		if (ch == '\\')
			ch = '/';
	}
	cmakelists << "include(\"" << flame_path << "cmake_utils.cmake\")\n";
	cmakelists << "add_definitions(-W0 -std:c++latest)\n";
	cmakelists << "file(GLOB SOURCE_LIST \"*.c*\")\n";
	cmakelists << "generate_rc()\n";
	cmakelists << "add_library(" << name << " SHARED ${SOURCE_LIST} \"${CMAKE_CURRENT_BINARY_DIR}/version.rc\")\n";
	cmakelists << "target_include_directories(" << name << " PRIVATE \"" << pugixml_path << "\")\n";
	cmakelists << "target_include_directories(" << name << " PRIVATE \"" << flame_path << "include\")\n";
	for (auto& l : libraries)
		cmakelists << "target_link_libraries(" << name << " \"" << l << "\")\n";
	cmakelists.close();

	std::wstring cmake_cmd(L"cmake ");
	cmake_cmd += L" -B build";
	exec_and_redirect_to_std_output(nullptr, cmake_cmd.data());

	exec_and_redirect_to_std_output(nullptr, wfmt(L"%s/Common7/IDE/devenv.com \"%s/build/bp.sln\" /build debug", s2w(VS_LOCATION).c_str(), get_curr_path().v).data());

	system("pause");

	return 0;
}
