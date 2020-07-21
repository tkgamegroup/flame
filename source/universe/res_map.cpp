#include <flame/serialize.h>
#include "res_map_private.h"

namespace flame
{
	std::filesystem::path ResMapPrivate::get_res_path(const std::string& name) const
	{
		auto it = res.find(name);
		assert(it != res.end());
		return it->second;
	}

	void ResMapPrivate::traversal(void (*callback)(Capture& c, const char* name, const wchar_t* path), const Capture& capture) const
	{
		for (auto& r : res)
			callback((Capture&)capture, r.first.c_str(), r.second.c_str());
	}

	void ResMapPrivate::load(const std::filesystem::path& filename)
	{
		parent_path = filename.parent_path();
		parent_path.make_preferred();

		auto ini = parse_ini_file(filename);
		for (auto& e : ini.get_section_entries(""))
		{
			auto add_res = [&](const std::string& k, const std::string& v) {
				assert(res.find(k) == res.end());
				auto path = parent_path / v;
				path.make_preferred();
				res.emplace(k, path);
			};
			auto star1 = std::count(e.key.begin(), e.key.end(), '*');
			auto star2 = std::count(e.value.begin(), e.value.end(), '*');
			if (star1 == 0 && star2 == 0)
				add_res(e.key, e.value);
			else if (star1 == 1 && star2 == 1)
			{
				auto r2str = std::string();
				for (auto ch : e.value)
				{
					if (ch == '*')
						r2str += "(.*)";
					else if (ch == '.')
						r2str += "\\.";
					else
						r2str += ch;
				}
				auto r1 = std::regex("\\*");
				auto r2 = std::regex(r2str);
				std::smatch res;
				for (std::filesystem::directory_iterator end, it(parent_path); it != end; it++)
				{
					auto str = it->path().filename().string();
					if (std::regex_search(str, res, r2))
						add_res(std::regex_replace(e.key, r1, res[1].str()), str);
				}
			}
		}
	}

	ResMap* ResMap::create()
	{
		return new ResMapPrivate();
	}
}
