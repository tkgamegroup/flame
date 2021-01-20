#include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/graphics/model.h>
#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "terrain_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cTerrainPrivate::set_blocks(const uvec2& b)
	{
		if (blocks == b)
			return;
		blocks = b;
		if (node)
			node->mark_transform_dirty();
	}

	void cTerrainPrivate::set_scale(const vec3& s)
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

	void cTerrainPrivate::set_height_map(const std::string& name)
	{
		if (height_map_name == name)
			return;
		height_map_name = name;
	}

	void cTerrainPrivate::set_normal_map(const std::string& name)
	{
		if (normal_map_name == name)
			return;
		normal_map_name = name;
	}

	void cTerrainPrivate::set_material_name(const std::string& name)
	{
		if (material_name == name)
			return;
		material_name = name;
	}

	void cTerrainPrivate::draw(graphics::Canvas* canvas)
	{
		if (height_map_id != -1 && normal_map_id != -1 && material_id != -1)
			canvas->draw_terrain(blocks, scale, node->g_pos, tess_levels, height_map_id, normal_map_id, material_id);
	}

	void cTerrainPrivate::on_added()
	{
		node = entity->get_component_t<cNodePrivate>();
		fassert(node);

		drawer = node->add_drawer([](Capture& c, graphics::Canvas* canvas) {
			auto thiz = c.thiz<cTerrainPrivate>();
			thiz->draw(canvas);
		}, Capture().set_thiz(this));
		node->mark_drawing_dirty();
	}

	void cTerrainPrivate::on_removed()
	{
		node->remove_drawer(drawer);
		node = nullptr;
	}

	void cTerrainPrivate::on_entered_world()
	{
		canvas = entity->world->get_system_t<sRendererPrivate>()->canvas;
		fassert(canvas);

		{
			auto isfile = false;
			auto fn = std::filesystem::path(height_map_name);
			if (!fn.extension().empty())
			{
				isfile = true;
				if (!fn.is_absolute())
					fn = (path.empty() ? entity->path.parent_path() : path) / fn;
			}
			height_map_id = canvas->find_texture_resource(fn.string().c_str());
			if (height_map_id == -1 && isfile)
			{
				auto t = graphics::Image::create(graphics::Device::get_default(), fn.c_str(), false, graphics::ImageUsageTransferSrc);
				height_map_id = canvas->set_texture_resource(-1, t->get_view(0), nullptr, fn.string().c_str());
			}
		}
		{
			auto isfile = false;
			auto fn = std::filesystem::path(normal_map_name);
			if (!fn.extension().empty())
			{
				isfile = true;
				if (!fn.is_absolute())
					fn = (path.empty() ? entity->path.parent_path() : path) / fn;
			}
			normal_map_id = canvas->find_texture_resource(fn.string().c_str());
			if (normal_map_id == -1 && isfile)
			{
				auto t = graphics::Image::create(graphics::Device::get_default(), fn.c_str(), false);
				normal_map_id = canvas->set_texture_resource(-1, t->get_view(0), nullptr, fn.string().c_str());
			}
		}
		{
			auto isfile = false;
			auto fn = std::filesystem::path(material_name);
			if (!fn.extension().empty())
			{
				isfile = true;
				if (!fn.is_absolute())
					fn = (path.empty() ? entity->path.parent_path() : path) / fn;
			}
			material_id = canvas->find_material_resource(fn.string().c_str());
			if (material_id == -1 && isfile)
			{
				auto m = graphics::Material::create(fn.c_str());
				material_id = canvas->set_material_resource(-1, m, fn.string().c_str());
			}
		}
	}

	void cTerrainPrivate::on_left_world()
	{
		canvas = nullptr;
	}

	cTerrain* cTerrain::create()
	{
		return f_new<cTerrainPrivate>();
	}
}
