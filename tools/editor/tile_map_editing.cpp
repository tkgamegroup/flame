#include "app.h"
#include "selection.h"
#include "scene_window.h"
#include "tile_map_editing.h"

#include <flame/universe/components/node.h>
#include <flame/universe/components/tile_map.h>

void tile_map_editing()
{
	auto fv = scene_window.first_view();
	if (!fv)
		return;

	auto& io = ImGui::GetIO();

	auto tile_map = selection.as_entity()->get_component<cTileMap>();
	auto extent = tile_map->extent;
	auto blocks = tile_map->blocks;
	auto block_sz = extent / vec3(blocks);

	auto pos = fv->hovering_pos - tile_map->node->global_pos();
	pos = round(pos / extent * (vec3)blocks);
	auto x = (int)pos.x; 
	auto z = (int)pos.z;
	if (x >= 0 && x <= blocks.x && z >= 0 && z <= blocks.z)
	{
		auto idx = x + z * (blocks.x + 1);
		auto sp = tile_map->samples[idx];
		auto c = vec3(x, sp.height * 0.5f + 0.1f, z) * block_sz;
		vec3 pts[5];
		pts[0] = c + vec3(-0.5f, 0.f, -0.5f);
		pts[1] = c + vec3(+0.5f, 0.f, -0.5f);
		pts[2] = c + vec3(+0.5f, 0.f, +0.5f);
		pts[3] = c + vec3(-0.5f, 0.f, +0.5f);
		pts[4] = c + vec3(-0.5f, 0.f, -0.5f);
		sRenderer::instance()->draw_primitives(PrimitiveLineStrip, pts, 5, cvec4(255, 128, 255, 255));

		switch (fv->tool)
		{
		case ToolTileMapLevelUp:
			if (io.MouseClicked[ImGuiMouseButton_Left])
			{
				if (sp.height < 255)
				{
					sp.height++;
					tile_map->set_sample(idx, sp);
					app.prefab_unsaved = true;
				}
			}
			break;
		case ToolTileMapLevelDown:
			if (io.MouseClicked[ImGuiMouseButton_Left])
			{
				if (sp.height > 0)
				{
					sp.height--;
					tile_map->set_sample(idx, sp);
					app.prefab_unsaved = true;
				}
			}
			break;
		case ToolTileMapSlope:
			if (io.MouseClicked[ImGuiMouseButton_Left])
			{
				if (x > 0 && x < blocks.x && z > 0 && z < blocks.z)
				{
					if (sp.slope == cTileMap::SlopeNone)
					{
						auto l = tile_map->samples[x - 1 + z * (blocks.x + 1)];
						auto r = tile_map->samples[x + 1 + z * (blocks.x + 1)];
						auto t = tile_map->samples[x + (z - 1) * (blocks.x + 1)];
						auto b = tile_map->samples[x + (z + 1) * (blocks.x + 1)];

						if ((l.height == r.height + 1 || r.height == l.height + 1) &&
							(l.slope == cTileMap::SlopeNone && r.slope == cTileMap::SlopeNone) &&
							!(t.height == b.height + 1 || b.height == t.height + 1))
						{
							sp.slope = cTileMap::SlopeHorizontal;
							tile_map->set_sample(idx, sp);
							app.prefab_unsaved = true;
						}
						else if ((t.height == b.height + 1 || b.height == t.height + 1) &&
							(t.slope == cTileMap::SlopeNone && b.slope == cTileMap::SlopeNone) &&
							!(l.height == r.height + 1 || r.height == l.height + 1))
						{
							sp.slope = cTileMap::SlopeVertical;
							tile_map->set_sample(idx, sp);
							app.prefab_unsaved = true;
						}
					}
				}
			}
			break;
		case ToolTileMapFlat:
			if (io.MouseClicked[ImGuiMouseButton_Left])
			{
				if (sp.slope != cTileMap::SlopeNone)
				{
					sp.slope = cTileMap::SlopeNone;
					tile_map->set_sample(idx, sp);
					app.prefab_unsaved = true;
				}
			}
			break;
		}
	}
}
