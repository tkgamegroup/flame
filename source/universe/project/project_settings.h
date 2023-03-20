#pragma once

#include "../universe.h"

namespace flame
{
	// Reflect
	struct ProjectSettings
	{
		// Reflect
		int build_after_open = 1; // 0 - no, 1 - yes, 2 - ask
		// Reflect
		std::vector<std::filesystem::path> favorites;

		std::filesystem::path filename;

		void clear()
		{
			favorites.clear();
		}

		FLAME_UNIVERSE_API void load(const std::filesystem::path& filename);
		FLAME_UNIVERSE_API void save();
	};

}
