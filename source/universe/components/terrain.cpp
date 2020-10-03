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

	void cTerrainPrivate::set_size(const Vec2u& s)
	{
		if (size == s)
			return;
		size = s;
		if (node)
			node->mark_transform_dirty();
	}

	void cTerrainPrivate::set_extent(const Vec3f& e)
	{
		if (extent == e)
			return;
		extent = e;
		if (node)
			node->mark_transform_dirty();
	}

	void cTerrainPrivate::set_tess_levels(float l)
	{
		if (tess_levels == l)
			return;
		tess_levels = l;
		if (node)
			node->mark_transform_dirty();
	}

	void cTerrainPrivate::set_blend_map(const char* name)
	{
		if (blend_map_name == name)
			return;
		blend_map_name = name;
	}

	void cTerrainPrivate::set_color_map_0(const char* name)
	{
		if (color_map_names[0] == name)
			return;
		color_map_names[0] = name;
	}

	void cTerrainPrivate::set_color_map_1(const char* name)
	{
		if (color_map_names[1] == name)
			return;
		color_map_names[1] = name;
	}

	void cTerrainPrivate::set_color_map_2(const char* name)
	{
		if (color_map_names[2] == name)
			return;
		color_map_names[2] = name;
	}

	void cTerrainPrivate::set_color_map_3(const char* name)
	{
		if (color_map_names[3] == name)
			return;
		color_map_names[3] = name;
	}

	void cTerrainPrivate::on_gain_canvas()
	{
		height_map_id = canvas->find_resource(graphics::ResourceTexture, height_map_name.c_str());
		blend_map_id = canvas->find_resource(graphics::ResourceTexture, blend_map_name.c_str());
		for (auto i = 0; i < 4; i++)
			color_map_ids[i] = canvas->find_resource(graphics::ResourceTexture, color_map_names[i].c_str());
	}

	void cTerrainPrivate::draw(graphics::Canvas* canvas)
	{
		if (height_map_id != -1)
			canvas->draw_terrain(height_map_id, size, extent, node->global_pos, tess_levels, blend_map_id, color_map_ids[0], color_map_ids[1], color_map_ids[2], color_map_ids[3]);
	}

	cTerrain* cTerrain::create()
	{
		return f_new<cTerrainPrivate>();
	}
}
