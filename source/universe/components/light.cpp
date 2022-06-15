#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "light_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	cLightPrivate::~cLightPrivate()
	{
		node->drawers.remove("light"_h);
		node->measurers.remove("light"_h);
	}

	void cLightPrivate::on_init()
	{
		node->drawers.add([this](sRendererPtr renderer, uint pass) {
			if (pass == "light"_h)
				draw(renderer);
		}, "light"_h);

		node->measurers.add([this](AABB* ret) {
			switch (type)
			{
			case LightDirectional:
				*ret = AABB(node->g_pos - 10000.f, node->g_pos + 10000.f);
				return true;
			case LightPoint:
				*ret = AABB(node->g_pos - range, node->g_pos + range);
				return true;
			case LightSpot:
				*ret = AABB(node->g_pos - range, node->g_pos + range);
				return true;
			}
			return false;
		}, "light"_h);

		node->mark_transform_dirty();
	}

	void cLightPrivate::set_type(LightType _type)
	{
		if (type == _type)
			return;
		type = _type;

		node->mark_transform_dirty();

		data_changed("type"_h);
	}

	void cLightPrivate::set_color(const vec4& _color)
	{
		if (color == _color)
			return;
		color = _color;

		node->mark_drawing_dirty();

		data_changed("color"_h);
	}

	void cLightPrivate::set_range(float _range)
	{
		if (range == _range)
			return;
		range = _range;

		node->mark_transform_dirty();

		data_changed("range"_h);
	}

	void cLightPrivate::set_cast_shadow(bool _cast_shadow)
	{
		if (cast_shadow == _cast_shadow)
			return;
		cast_shadow = _cast_shadow;

		node->mark_drawing_dirty();

		data_changed("cast_shadow"_h);
	}

	void cLightPrivate::draw(sRendererPtr renderer)
	{
		renderer->add_light(instance_id, type, type == LightDirectional ? node->g_rot[2] : node->g_pos, 
			color.rgb() * color.a, range, cast_shadow);
	}

	void cLightPrivate::on_active()
	{
		instance_id = sRenderer::instance()->register_light_instance(-1);
	}

	void cLightPrivate::on_inactive()
	{
		sRenderer::instance()->register_light_instance(instance_id);
		instance_id = -1;
	}

	struct cLightCreate : cLight::Create
	{
		cLightPtr operator()(EntityPtr e) override
		{
			return new cLightPrivate();
		}
	}cLight_create;
	cLight::Create& cLight::create = cLight_create;
}
