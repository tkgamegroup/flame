#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "dir_light_private.h"
#include "../draw_data.h"
#include "../systems/renderer_private.h"

namespace flame
{
	cDirLightPrivate::~cDirLightPrivate()
	{
		node->drawers.remove("light"_h);
		node->measurers.remove("light"_h);
		node->data_listeners.remove("light"_h);
	}

	void cDirLightPrivate::on_init()
	{
		node->drawers.add([this](DrawData& draw_data) {
			if (instance_id == -1)
				return;

			switch (draw_data.pass)
			{
			case "instance"_h:
				if (dirty)
				{
					sRenderer::instance()->set_dir_light_instance(instance_id, node->g_rot[2], color.rgb() * color.a);
					dirty = false;
				}
				break;
			case "light"_h:
				draw_data.lights.emplace_back(LightDirectional, instance_id, cast_shadow);
			}
		}, "light"_h);

		node->measurers.add([this](AABB* ret) {
			*ret = AABB(node->g_pos - 10000.f, node->g_pos + 10000.f);
			return true;
		}, "light"_h);
		node->data_listeners.add([this](uint hash) {
			if (hash == "transform"_h)
				dirty = true;
		}, "light"_h);

		node->mark_transform_dirty();
	}

	void cDirLightPrivate::set_color(const vec4& _color)
	{
		if (color == _color)
			return;
		color = _color;

		dirty = true;
		node->mark_drawing_dirty();
		data_changed("color"_h);
	}

	void cDirLightPrivate::set_cast_shadow(bool _cast_shadow)
	{
		if (cast_shadow == _cast_shadow)
			return;
		cast_shadow = _cast_shadow;

		dirty = true;
		node->mark_drawing_dirty();
		data_changed("cast_shadow"_h);
	}

	void cDirLightPrivate::on_active()
	{
		instance_id = sRenderer::instance()->register_light_instance(LightDirectional, -1);
	}

	void cDirLightPrivate::on_inactive()
	{
		sRenderer::instance()->register_light_instance(LightDirectional, instance_id);
		instance_id = -1;
	}

	struct cDirLightCreate : cDirLight::Create
	{
		cDirLightPtr operator()(EntityPtr e) override
		{
			return new cDirLightPrivate();
		}
	}cDirLight_create;
	cDirLight::Create& cDirLight::create = cDirLight_create;
}
