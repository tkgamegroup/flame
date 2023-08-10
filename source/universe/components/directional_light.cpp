#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "directional_light_private.h"
#include "../draw_data.h"
#include "../systems/renderer_private.h"

namespace flame
{
	cDirectionalLightPrivate::~cDirectionalLightPrivate()
	{
		node->drawers.remove("light"_h);
		node->measurers.remove("light"_h);
		node->data_listeners.remove("light"_h);
	}

	void cDirectionalLightPrivate::on_init()
	{
		node->drawers.add([this](DrawData& draw_data) {
			if (instance_id == -1)
				return;

			switch (draw_data.pass)
			{
			case PassInstance:
				if (dirty)
				{
					sRenderer::instance()->set_dir_light_instance(instance_id, node->z_axis(), color.rgb() * color.a);
					dirty = false;
				}
				break;
			case PassLight:
				draw_data.lights.emplace_back(LightDirectional, instance_id, cast_shadow);
			}
		}, "light"_h);

		node->measurers.add([this](AABB& b) {
			b.expand(AABB(node->global_pos(), 10000.f));
		}, "light"_h);
		node->data_listeners.add([this](uint hash) {
			if (hash == "transform"_h)
				dirty = true;
		}, "light"_h);

		node->mark_transform_dirty();
	}

	void cDirectionalLightPrivate::set_color(const vec4& _color)
	{
		if (color == _color)
			return;
		color = _color;

		dirty = true;
		node->mark_drawing_dirty();
		data_changed("color"_h);
	}

	void cDirectionalLightPrivate::set_cast_shadow(bool _cast_shadow)
	{
		if (cast_shadow == _cast_shadow)
			return;
		cast_shadow = _cast_shadow;

		dirty = true;
		node->mark_drawing_dirty();
		data_changed("cast_shadow"_h);
	}

	void cDirectionalLightPrivate::on_active()
	{
		instance_id = sRenderer::instance()->register_light_instance(LightDirectional, -1);
	}

	void cDirectionalLightPrivate::on_inactive()
	{
		sRenderer::instance()->register_light_instance(LightDirectional, instance_id);
		instance_id = -1;
	}

	struct cDirectionalLightCreate : cDirectionalLight::Create
	{
		cDirectionalLightPtr operator()(EntityPtr e) override
		{
			return new cDirectionalLightPrivate();
		}
	}cDirectionalLight_create;
	cDirectionalLight::Create& cDirectionalLight::create = cDirectionalLight_create;
}
