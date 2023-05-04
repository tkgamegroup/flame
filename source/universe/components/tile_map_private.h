#pragma once

#include "tile_map.h"

namespace flame
{
	struct cTileMapPrivate : cTileMap
	{
		struct Mesh
		{
			std::filesystem::path path;
			uint rotation = 0;

			Mesh() = default;
			Mesh(const std::filesystem::path& path, uint rotation) :
				path(path),
				rotation(rotation)
			{
			}
		};

		std::map<std::string, Mesh> meshes;
		bool dirty = true;

		void set_extent(const vec3& extent) override;
		void set_blocks(const uvec3& blocks) override;
		void set_tiles_path(const std::filesystem::path& path) override;
		void set_samples(const std::vector<Sample>& samples) override;
		void set_sample(uint idx, const Sample& v) override;

		void update_tiles();

		void on_active() override;
		void on_inactive() override;
	};
}
