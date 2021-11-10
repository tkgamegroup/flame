#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

using namespace flame;

auto copied_files_count = 0;

std::string config;

int main(int argc, char **args)
{
	std::filesystem::path engine_path = getenv("FLAME_PATH");
	std::filesystem::path src;
	std::filesystem::path dst;
	std::vector<std::string> engine_items;
	std::vector<std::string> items;

	config = args[1];
	if (config == "d")
		config = "debug";
	else if (config == "r")
		config = "relwithdebinfo";
	std::transform(config.begin(), config.end(), config.begin(), ::tolower);

	auto replace_config_str = [](std::string_view s) {
		static auto reg = std::regex(R"(\{c\})");
		return std::regex_replace(s, reg, config);
	};
	auto description = parse_ini_file(L"package_description.ini");
	for (auto& e : description.get_section_entries(""))
	{
		if (e.key == "src")
			src = replace_config_str(e.value);
		else if (e.key == "dst")
			dst = replace_config_str(e.value);
	}
	for (auto& e : description.get_section_entries("engine_items"))
		engine_items.push_back(replace_config_str(e.value));
	for (auto& e : description.get_section_entries("items"))
		items.push_back(replace_config_str(e.value));

	auto copied_item_count = 0;
	auto copy_item = [&](const std::filesystem::path& b, std::string_view i) {
		auto s = b / i;
		auto d = dst / i;

		auto ext = s.extension();
		if (ext == L".exe" || ext == L".dll")
			d = dst / std::filesystem::path(i).filename();

		auto s_p = s.parent_path();
		auto d_p = d.parent_path();

		if (!std::filesystem::exists(d_p))
			std::filesystem::create_directories(d_p);
		std::filesystem::copy_file(s, d, std::filesystem::copy_options::overwrite_existing);
		wprintf(L"%s   =>   %s\n", s.c_str(), d.c_str());
		copied_item_count++;
		copied_files_count++;

		auto copy_binary_attachings = [](const std::filesystem::path& _s, const std::filesystem::path& _d) {
			auto s = _s;
			s.replace_extension(L".typeinfo");
			if (std::filesystem::exists(s))
			{
				auto d = _d;
				d.replace_extension(L".typeinfo");
				std::filesystem::copy_file(s, d, std::filesystem::copy_options::overwrite_existing);
				wprintf(L"%s   =>   %s\n", s.c_str(), d.c_str());
				copied_files_count++;
			}
			if (config == "debug")
			{
				auto s = _s;
				s.replace_extension(L".pdb");
				if (std::filesystem::exists(s))
				{
					auto d = _d;
					d.replace_extension(L".pdb");
					std::filesystem::copy_file(s, d, std::filesystem::copy_options::overwrite_existing);
					wprintf(L"%s   =>   %s\n", s.c_str(), d.c_str());
					copied_files_count++;
				}
			}
		};

		if (ext == L".exe" || ext == L".dll")
		{
			copy_binary_attachings(s, d);
			std::vector<std::filesystem::path> dependencies;
			get_module_dependencies(s.c_str(), [](Capture& c, const wchar_t* filename) {
				auto& dependencies = *c.thiz<std::vector<std::filesystem::path>>();
				dependencies.push_back(filename);
			}, Capture().set_thiz(&dependencies));
			for (auto& d : dependencies)
			{
				if (d.wstring().starts_with(L"flame_"))
				{
					auto ss = s_p / d;
					auto dd = d_p / d;
					std::filesystem::copy_file(ss, dd, std::filesystem::copy_options::overwrite_existing);
					wprintf(L"%s   =>   %s\n", ss.c_str(), dd.c_str());
					copied_files_count++;
					copy_binary_attachings(ss, dd);
				}
			}
		}
		else if (ext == L".atlas")
		{
			auto atlas = parse_ini_file(s);
			for (auto& e : atlas.get_section_entries(""))
			{
				if (e.key == "image")
				{
					auto ss = s_p / e.value;
					if (std::filesystem::exists(ss))
					{
						auto dd = d_p / e.value;
						std::filesystem::copy_file(ss, dd, std::filesystem::copy_options::overwrite_existing);
						wprintf(L"%s   =>   %s\n", ss.c_str(), dd.c_str());
						copied_files_count++;
					}
					break;
				}
			}
		}
		else if (ext == L".spv")
		{
			auto ss = s;
			ss.replace_extension(L".res");
			if (std::filesystem::exists(ss))
			{
				auto dd = d;
				dd.replace_extension(L".res");
				std::filesystem::copy_file(ss, dd, std::filesystem::copy_options::overwrite_existing);
				wprintf(L"%s   =>   %s\n", ss.c_str(), dd.c_str());
				copied_files_count++;
			}
		}
	};
	for (auto& i : engine_items)
		copy_item(engine_path, i);
	for (auto& i : items)
		copy_item(src, i);

	printf("copied: %d/%d items, %d files\n", copied_item_count, engine_items.size() + items.size(), copied_files_count);

	return 0;
}
