#include <flame/foundation/foundation.h>
#include <flame/foundation/system.h>

using namespace flame;

int main(int argc, char** args)
{
	auto current_path = std::filesystem::current_path();
	auto project_path = current_path;
	if (!std::filesystem::exists(project_path / L"assets"))
		return 0;
	auto package_path = project_path / L"out";
	if (!std::filesystem::exists(package_path))
		std::filesystem::create_directories(package_path);
	std::filesystem::remove_all(package_path);
	std::filesystem::create_directories(project_path / L"out/flame");
	std::filesystem::create_directories(project_path / L"out/assets");

	if (auto dll_path = std::filesystem::path(L"C:/Windows/System32/ucrtbased.dll"); std::filesystem::exists(dll_path))
		std::filesystem::copy(dll_path, package_path / L"ucrtbased.dll");
	if (auto dll_path = std::filesystem::path(L"C:/Windows/System32/OpenAL32.dll"); std::filesystem::exists(dll_path))
		std::filesystem::copy(dll_path, package_path / L"OpenAL32.dll");
	//if (auto dll_path = std::filesystem::path(L"C:/Windows/System32/vulkan-1.dll"); std::filesystem::exists(dll_path))
	//	std::filesystem::copy(dll_path, package_path / L"vulkan-1.dll");
	if (auto vk_sdk_path = getenv("VK_SDK_PATH"); vk_sdk_path)
	{
		std::filesystem::path glslc_path = std::format(L"{}/Bin/glslc.exe", s2w(vk_sdk_path));
		if (std::filesystem::exists(glslc_path))
			std::filesystem::copy(glslc_path, package_path / L"glslc.exe");
	}

	auto vs_path = get_special_path("Visual Studio Installation Location");
	if (auto debug_crt_path = vs_path / L"VC\\Redist\\MSVC\\14.40.33807\\debug_nonredist\\x64\\Microsoft.VC143.DebugCRT"; std::filesystem::exists(debug_crt_path))
	{
		std::filesystem::copy(debug_crt_path / L"msvcp140d.dll", package_path);
		std::filesystem::copy(debug_crt_path / L"msvcp140_1d.dll", package_path);
		std::filesystem::copy(debug_crt_path / L"vcruntime140d.dll", package_path);
		std::filesystem::copy(debug_crt_path / L"vcruntime140_1d.dll", package_path);
	}

	for (auto& it : std::filesystem::directory_iterator(project_path / L"bin/debug"))
	{
		if (it.is_regular_file())
		{
			auto ext = it.path().extension();
			if (ext == L".dll" || ext == L".exe" /*|| ext == L".pdb"*/ || ext == L".typeinfo")
				std::filesystem::copy_file(it.path(), package_path / it.path().filename(), std::filesystem::copy_options::overwrite_existing);
		}
	}
	if (auto flame_path = getenv("FLAME_PATH"); flame_path)
	{
		auto flame_assets_path = std::filesystem::path(flame_path) / L"assets";
		std::filesystem::copy(flame_assets_path, project_path / L"out/flame", std::filesystem::copy_options::recursive);
	}
	std::filesystem::copy(project_path / L"assets", project_path / L"out/assets", std::filesystem::copy_options::recursive);
}
