#include <flame/graphics/canvas.h>
#include "node_private.h"
#include "terrain_private.h"

namespace flame
{
	void cTerrainPrivate::set_height_map(const std::string& name)
	{
		if (height_map_name == name)
			return;
		height_map_name = name;
	}

	void cTerrainPrivate::set_color_map(const char* name)
	{
		if (color_map_name == name)
			return;
		color_map_name = name;
	}

	void cTerrainPrivate::set_blocks(const Vec2u& b)
	{
		if (blocks == b)
			return;
		blocks = b;
		if (node)
			node->mark_transform_dirty();
	}

	void cTerrainPrivate::set_scale(const Vec3f& s)
	{
		if (scale == s)
			return;
		scale = s;
		if (node)
			node->mark_transform_dirty();
	}

	void cTerrainPrivate::set_tess_levels(uint l)
	{
		if (tess_levels == l)
			return;
		tess_levels = l;
		if (node)
			node->mark_transform_dirty();
	}

	void cTerrainPrivate::on_gain_canvas()
	{
		height_map_id = canvas->find_resource(graphics::ResourceTexture, height_map_name.c_str());
		color_map_id = canvas->find_resource(graphics::ResourceTexture, color_map_name.c_str());
	}

	void cTerrainPrivate::draw(graphics::Canvas* canvas)
	{
		if (height_map_id != -1 && color_map_id != -1)
			canvas->draw_terrain(height_map_id, color_map_id, blocks, scale, node->global_pos, tess_levels);
	}

	cTerrain* cTerrain::create()
	{
		return f_new<cTerrainPrivate>();
	}
}
