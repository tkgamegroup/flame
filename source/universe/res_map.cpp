#include <flame/serialize.h>
#include "res_map_private.h"

namespace flame
{
	ResMapPrivate::ResMapPrivate(const std::filesystem::path& filename)
	{
		parent_path = filename.parent_path();
		parent_path.make_preferred();

		auto ini = parse_ini_file(filename);
		for (auto& e : ini.get_section_entries(""))
		{
			assert(res.find(e.key) == res.end());
			auto path = parent_path / e.value;
			path.make_preferred();
			res.emplace(e.key, path);
		}
	}

	void ResMapBridge::get_res_path(const char* name, wchar_t* dst) const
	{
		auto path = ((ResMapPrivate*)this)->get_res_path(name).wstring();
		std::char_traits<wchar_t>::copy(dst, path.c_str(), path.size());
		dst[path.size()] = 0;
	}

	std::filesystem::path ResMapPrivate::get_res_path(const std::string& name) const
	{
		auto it = res.find(name);
		assert(it != res.end());
		return it->second;
	}

	ResMap* ResMap::create(const wchar_t* filename)
	{
		return new ResMapPrivate(filename);
	}
}
