#pragma once

#include "../universe.h"

namespace flame
{
	// Reflect
	struct ProjectSettings
	{
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
