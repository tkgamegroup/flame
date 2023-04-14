#include "tile_map_private.h"

namespace flame
{
	void cTileMapPrivate::set_extent(const vec3& extent)
	{

	}

	void cTileMapPrivate::set_blocks(const uvec3& blocks)
	{

	}

	void cTileMapPrivate::set_tiles_path(const std::filesystem::path& path)
	{

	}

	struct cTileMapCreate : cTileMap::Create
	{
		cTileMapPtr operator()(EntityPtr e) override
		{
			return new cTileMapPrivate();
		}
	}cTileMap_create;
	cTileMap::Create& cTileMap::create = cTileMap_create;
}
