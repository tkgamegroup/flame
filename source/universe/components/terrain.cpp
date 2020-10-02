#include <flame/graphics/canvas.h>
#include "node_private.h"
#include "terrain_private.h"

namespace flame
{
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

	void cTerrainPrivate::on_gain_canvas()
	{
		height_map_id = canvas->find_resource(graphics::ResourceTexture, height_map_name.c_str());
	}

	void cTerrainPrivate::draw(graphics::Canvas* canvas)
	{
		if (height_map_id != -1)
			canvas->draw_terrain(height_map_id, size, extent, node->global_pos, tess_levels);
	}

	cTerrain* cTerrain::create()
	{
		return f_new<cTerrainPrivate>();
	}
}
