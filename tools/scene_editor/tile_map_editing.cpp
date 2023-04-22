#include "app.h"
#include "selection.h"
#include "view_scene.h"
#include "tile_map_editing.h"

#include <flame/universe/components/node.h>
#include <flame/universe/components/tile_map.h>

void tile_map_editing()
{
	auto& io = ImGui::GetIO();

	auto tile_map = selection.as_entity()->get_component_t<cTileMap>();
	auto extent = tile_map->extent;
	auto blocks = tile_map->blocks;
	auto block_sz = extent / vec3(blocks);

	auto pos = view_scene.hovering_pos - tile_map->node->global_pos();
	pos = round(pos / extent * (vec3)blocks);
	auto x = (int)pos.x; 
	auto z = (int)pos.z;
	if (x >= 0 && x <= blocks.x && z >= 0 && z <= blocks.z)
	{
		auto sp = tile_map->samples[x + z * (blocks.x + 1)];
		auto c = vec3(x, sp * 0.5f + 0.1f, z) * block_sz;
		vec3 pts[5];
		pts[0] = c + vec3(-0.5f, 0.f, -0.5f);
		pts[1] = c + vec3(+0.5f, 0.f, -0.5f);
		pts[2] = c + vec3(+0.5f, 0.f, +0.5f);
		pts[3] = c + vec3(-0.5f, 0.f, +0.5f);
		pts[4] = c + vec3(-0.5f, 0.f, -0.5f);
		sRenderer::instance()->draw_primitives("LineStrip"_h, pts, 5, cvec4(255, 128, 255, 255));

		switch (app.tool)
		{
		case ToolTileMapLevelUp:
			if (io.MouseClicked[ImGuiMouseButton_Left])
				tile_map->add_sample(x + z * (blocks.x + 1), +2);
			break;
		case ToolTileMapLevelDown:
			if (io.MouseClicked[ImGuiMouseButton_Left])
				tile_map->add_sample(x + z * (blocks.x + 1), -2);
			break;
		case ToolTileMapSlope:
			if (io.MouseClicked[ImGuiMouseButton_Left])
			{
				if (x > 0 && x < blocks.x && z > 0 && z < blocks.z)
				{
					if (sp % 2 != 1)
					{
						auto l = tile_map->samples[x - 1 + z * (blocks.x + 1)];
						auto r = tile_map->samples[x + 1 + z * (blocks.x + 1)];
						if (l == r + 2 || r == l + 2)
						{
							tile_map->add_sample(x + z * (blocks.x + 1), +1);
							break;
						}
						auto t = tile_map->samples[x + (z - 1) * (blocks.x + 1)];
						auto b = tile_map->samples[x + (z + 1) * (blocks.x + 1)];
						if (t == b + 2 || b == t + 2)
						{
							tile_map->add_sample(x + z * (blocks.x + 1), +1);
							break;
						}
					}
				}
			}
			break;
		}
	}
}
