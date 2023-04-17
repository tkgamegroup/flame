#pragma once

#include "tile_map.h"

namespace flame
{
	struct cTileMapPrivate : cTileMap
	{
		std::vector<EntityPtr> meshes;

		void set_extent(const vec3& extent) override;
		void set_blocks(const uvec3& blocks) override;
		void set_tiles_path(const std::filesystem::path& path) override;

		void update_tiles();
	};
}
