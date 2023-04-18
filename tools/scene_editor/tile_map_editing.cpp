#include "app.h"
#include "selection.h"
#include "view_scene.h"
#include "tile_map_editing.h"

#include <flame/universe/components/node.h>
#include <flame/universe/components/tile_map.h>

void tile_map_editing()
{
	auto tile_map = selection.as_entity()->get_component_t<cTileMap>();
	auto extent = tile_map->extent;
	auto blocks = tile_map->blocks;

	auto pos = view_scene.hovering_pos - tile_map->node->global_pos();
	pos = round(pos / extent * (vec3)blocks);
	auto x = (int)pos.x; 
	auto z = (int)pos.z;

	auto& io = ImGui::GetIO();
	switch (app.tool)
	{
	case ToolTileMapLevelUp:
		if (io.MouseClicked[ImGuiMouseButton_Left])
		{
			if (x >= 0 && x <= blocks.x && z >= 0 && z <= blocks.z)
				tile_map->set_sample(x + z * (blocks.x + 1), 1);
		}
		break;
	case ToolTileMapLevelDown:
		if (io.MouseClicked[ImGuiMouseButton_Left])
		{
			if (x >= 0 && x <= blocks.x && z >= 0 && z <= blocks.z)
				tile_map->set_sample(x + z * (blocks.x + 1), 0);
		}
		break;
	}
}
