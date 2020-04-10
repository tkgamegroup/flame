#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

using namespace flame;

int main(int argc, char **args)
{
	auto action = std::string(args[1]);

	std::string name;
	std::vector<std::string> libraries;
	auto compile_otions = parse_ini_file(L"library_description.ini");
	for (auto& e : compile_otions.get_section_entries(""))
	{
		if (e.key == "name")
			name = e.value;
	}
	for (auto& e : compile_otions.get_section_entries("libraries"))
		libraries.push_back(e.value);

	auto flame_path = std::string(getenv("FLAME_PATH"));
	std::replace(flame_path.begin(), flame_path.end(), '\\', '/');

	std::ofstream cmakelists(L"CMakeLists.txt");
	cmakelists << "# THIS FILE IS AUTO GENERATED\n";
	cmakelists << "cmake_minimum_required(VERSION 3.16.4)\n";
	cmakelists << "project(" << name << ")\n";
	cmakelists << "include(\"" << flame_path << "cmake_utils.cmake\")\n";
	cmakelists << "add_definitions(-W0 -std:c++latest)\n";
	cmakelists << "file(GLOB SOURCE_LIST \"*.h*\" \"*.c*\")\n";
	cmakelists << "generate_rc()\n";
	cmakelists << "add_library(" << name << " SHARED ${SOURCE_LIST} \"${CMAKE_CURRENT_BINARY_DIR}/version.rc\")\n";
	cmakelists << "target_include_directories(" << name << " PRIVATE \"" << flame_path << "include\")\n";
	for (auto& l : libraries)
	{
		cmakelists << "target_link_libraries(" << name << " debug \"" << flame_path << "bin/debug/" << l << ".lib\")\n";
		cmakelists << "target_link_libraries(" << name << " optimized \"" << flame_path << "bin/relwithdebinfo/" << l << ".lib\")\n";
	}

	{
		// reflect the setup about xml and json libraries

		std::ifstream cmake_caches(flame_path + "build/CMakeCache.txt");
		while (!cmake_caches.eof())
		{
			std::string line;
			std::getline(cmake_caches, line);
			static std::regex reg_pugixml_include_dir(R"(PUGIXML_INCLUDE_DIR:PATH\=(.*))");
			static std::regex reg_pugixml_debug_static_library_path(R"(PUGIXML_DEBUG_STATIC_LIBRARY_PATH:FILEPATH\=(.*))");
			static std::regex reg_pugixml_release_static_library_path(R"(PUGIXML_RELEASE_STATIC_LIBRARY_PATH:FILEPATH\=(.*))");
			static std::regex reg_njson_include_dir(R"(NJSON_INCLUDE_DIR:PATH\=(.*))");
			std::smatch res;
			if (std::regex_search(line, res, reg_pugixml_include_dir))
				cmakelists << "target_include_directories(" << name << " PRIVATE \"" << res[1].str() << "\")\n";
			else if (std::regex_search(line, res, reg_pugixml_debug_static_library_path))
				cmakelists << "target_link_libraries(" << name << " debug \"" << res[1].str() << "\")\n";
			else if (std::regex_search(line, res, reg_pugixml_release_static_library_path))
				cmakelists << "target_link_libraries(" << name << " optimized \"" << res[1].str() << "\")\n";
			else if (std::regex_search(line, res, reg_njson_include_dir))
				cmakelists << "target_include_directories(" << name << " PRIVATE \"" << res[1].str() << "\")\n";
		}
		cmake_caches.close();
		cmakelists.close();
	}

	std::wstring vs_path = s2w(VS_LOCATION);

	auto cmake_cmd = L"\"" + vs_path + L"/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe\" ";
	cmake_cmd += L" -B build";
	exec_and_redirect_to_std_output(nullptr, cmake_cmd.data());

	if (action != "p")
	{

		auto config = L"";
		if (action == "d")
			config = L"debug";
		else if (action == "r")
			config = L"relwithdebinfo";
		exec_and_redirect_to_std_output(nullptr, wfmt(L"%s/Common7/IDE/devenv.com \"%s/build/%s.sln\" /build %s", vs_path.c_str(), get_curr_path().v, s2w(name).c_str(), config).data());
	}

	system("pause");

	return 0;
}
