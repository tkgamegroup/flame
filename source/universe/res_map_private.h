#pragma once

#include <flame/universe/res_map.h>

namespace flame
{
	struct ResMapBridge : ResMap
	{
		void get_res_path(const char* name, wchar_t* dst) const override;
	};

	struct ResMapPrivate : ResMapBridge
	{
		std::filesystem::path parent_path;
		std::unordered_map<std::string, std::filesystem::path> res;

		ResMapPrivate(const std::filesystem::path& filename);

		void release() override { delete this; };

		std::filesystem::path get_res_path(const std::string& name) const;
	};
}
