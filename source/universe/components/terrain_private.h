#pragma once

#include "terrain.h"
#include "node_private.h"

namespace flame
{
	struct cTerrainPrivate : cTerrain
	{
		int frame = -1;

		~cTerrainPrivate();
		void on_init() override;

		void set_extent(const vec3& extent) override;
		void set_blocks(const uvec2& blocks) override;
		void set_tess_level(uint tess_level) override;
		void set_texture_name(const std::filesystem::path& texture_name) override;

		void apply_src();

		void draw(sRendererPtr renderer);

		void on_active() override;
		void on_inactive() override;
	};
}
