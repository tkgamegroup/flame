#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "pt_light_private.h"
#include "../draw_data.h"
#include "../systems/renderer_private.h"

namespace flame
{
	cPtLightPrivate::~cPtLightPrivate()
	{
		node->drawers.remove("light"_h);
		node->measurers.remove("light"_h);
	}

	void cPtLightPrivate::on_init()
	{
		node->drawers.add([this](DrawData& draw_data) {
			if (instance_id == -1)
				return;

			switch (draw_data.pass)
			{
			case "instance"_h:
				sRenderer::instance()->set_pt_light_instance(instance_id, node->g_pos, color.rgb() * color.a, range);
				break;
			case "light"_h:
				draw_data.lights.emplace_back(LightPoint, instance_id, cast_shadow);
			}
		}, "light"_h);

		node->measurers.add([this](AABB* ret) {
			*ret = AABB(node->g_pos - range, node->g_pos + range);
			return true;
		}, "light"_h);

		node->mark_transform_dirty();
	}

	void cPtLightPrivate::set_color(const vec4& _color)
	{
		if (color == _color)
			return;
		color = _color;

		node->mark_drawing_dirty();

		data_changed("color"_h);
	}

	void cPtLightPrivate::set_range(float _range)
	{
		if (range == _range)
			return;
		range = _range;

		node->mark_transform_dirty();

		data_changed("range"_h);
	}

	void cPtLightPrivate::set_cast_shadow(bool _cast_shadow)
	{
		if (cast_shadow == _cast_shadow)
			return;
		cast_shadow = _cast_shadow;

		node->mark_drawing_dirty();

		data_changed("cast_shadow"_h);
	}

	void cPtLightPrivate::on_active()
	{
		instance_id = sRenderer::instance()->register_light_instance(LightPoint, -1);
	}

	void cPtLightPrivate::on_inactive()
	{
		sRenderer::instance()->register_light_instance(LightPoint, instance_id);
		instance_id = -1;
	}

	struct cPtLightCreate : cPtLight::Create
	{
		cPtLightPtr operator()(EntityPtr e) override
		{
			return new cPtLightPrivate();
		}
	}cPtLight_create;
	cPtLight::Create& cPtLight::create = cPtLight_create;
}
