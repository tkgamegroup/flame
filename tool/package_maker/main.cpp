#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

using namespace flame;

auto copied_files_count = 0;

void copy_typeinfo(const std::filesystem::path& _s, const std::filesystem::path& _d)
{
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
}

void copy_pdb(const std::filesystem::path& _s, const std::filesystem::path& _d)
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

int main(int argc, char **args)
{
	std::filesystem::path engine_dir = getenv("FLAME_PATH");
	std::filesystem::path source;
	std::filesystem::path destination;
	std::vector<std::string> items;

	std::string config = args[1];
	std::transform(config.begin(), config.end(), config.begin(), ::tolower);

	auto description = parse_ini_file(L"package_description.ini");
	for (auto& e : description.get_section_entries(""))
	{
		auto fmt = [&config](std::string& s) {
			static FLAME_SAL(str, "{c}");
			auto pos = s.find(str.s, 0, str.l);
			while (pos != std::string::npos)
			{
				s = s.replace(pos, str.l, config);
				pos = s.find(str.s, 0, str.l);
			}
		};
		if (e.key == "src")
		{
			auto str = e.value;
			fmt(str);
			source = str;
		}
		else if (e.key == "dst")
		{
			auto str = e.value;
			fmt(str);
			destination = str;
		}
	}
	for (auto& e : description.get_section_entries("items"))
		items.push_back(e.value);

	auto copied_item_count = 0;
	for (auto& i : items)
	{
		auto sp = SUS::split(i);
		std::filesystem::path s;
		std::filesystem::path d;
		std::filesystem::path p;
		std::filesystem::path n;

		{
			auto _sp = SUS::split(sp[0], '/');
			for (auto j = 0; j < _sp.size() - 1; j++)
			{
				if (_sp[j].size() == 3 && _sp[j][0] == '{' && _sp[j][2] == '}')
				{
					std::wstring str;
					switch (_sp[j][1])
					{
					case 'e':
						str = engine_dir;
						break;
					case 's':
						str = source;
						break;
					case 'c':
						str = s2w(config);
						break;
					}
					s /= str;
					if (j > 0)
						p /= str;
				}
				else
				{
					s /= _sp[j];
					p /= _sp[j];
				}
			}
			n = _sp.back();
			s /= n;
		}

		if (!std::filesystem::exists(s))
		{
			printf("cannot find src file: %s\n", s.string().c_str());
			continue;
		}

		{
			auto _sp = SUS::split(sp[1], '/');
			for (auto j = 0; j < _sp.size(); j++)
			{
				if (_sp[j].size() == 3 && _sp[j][0] == '{' && _sp[j][2] == '}')
				{
					switch (_sp[j][1])
					{
					case 'd':
						d /= destination;
						break;
					case 'p':
						d /= p;
						break;
					case 'n':
						d /= n;
						break;
					}
				}
				else
					d /= _sp[j];
			}
		}

		auto s_p = s.parent_path();
		auto d_p = d.parent_path();

		if (!std::filesystem::exists(d_p))
			std::filesystem::create_directories(d_p);
		std::filesystem::copy_file(s, d, std::filesystem::copy_options::overwrite_existing);
		wprintf(L"%s   =>   %s\n", s.c_str(), d.c_str());
		copied_item_count++;
		copied_files_count++;

		auto ext = n.extension();

		if (ext == L".exe" || ext == L".dll")
		{
			copy_typeinfo(s, d);
			if (config == "debug")
				copy_pdb(s, d);
			auto dependencies = get_module_dependencies(s.c_str());
			for (auto i = 0; i < dependencies.s; i++)
			{
				auto dep = dependencies[i].str();
				auto ss = s_p / dep;
				if (std::filesystem::exists(ss))
				{
					auto dd = d_p / dep;
					std::filesystem::copy_file(ss, dd, std::filesystem::copy_options::overwrite_existing);
					wprintf(L"%s   =>   %s\n", ss.c_str(), dd.c_str());
					copied_files_count++;
					copy_typeinfo(ss, dd);
					if (config == "debug")
						copy_pdb(ss, dd);
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
	}

	printf("copied: %d/%d items, %d files\n", copied_item_count, items.size(), copied_files_count);

	return 0;
}
