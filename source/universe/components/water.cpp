#include "../../graphics/device.h"
#include "../../graphics/material.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "water_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cWaterPrivate::set_extent(const vec2& ext)
	{
		if (extent == ext)
			return;
		extent = ext;
		if (node)
			node->mark_transform_dirty();
	}

	void cWaterPrivate::set_material_name(std::string_view name)
	{
		if (material_name == name)
			return;
		material_name = name;
	}

	void cWaterPrivate::draw(sRendererPtr s_renderer, bool first, bool)
	{
		if (!first || material_id == -1)
			return;

		s_renderer->draw_water(node->g_pos, extent, material_id, shading_flags);
	}

	void cWaterPrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		assert(node);

		node->mark_drawing_dirty();
	}

	void cWaterPrivate::on_removed()
	{
		node = nullptr;
	}

	void cWaterPrivate::on_entered_world()
	{
		s_renderer = entity->world->get_system_t<sRendererPrivate>();
		assert(s_renderer);

		auto ppath = entity->get_src(src_id).parent_path();

		if (!material_name.empty())
		{
			auto fn = std::filesystem::path(material_name);
			if (!fn.extension().empty() && !fn.is_absolute())
				fn = ppath / fn;
			material = graphics::Material::get(fn.c_str());
			material_id = s_renderer->find_material_res(material);
			if (material_id == -1)
				material_id = s_renderer->set_material_res(-1, material);
		}
	}

	void cWaterPrivate::on_left_world()
	{
		s_renderer = nullptr;
	}

	cWater* cWater::create()
	{
		return new cWaterPrivate();
	}
}
