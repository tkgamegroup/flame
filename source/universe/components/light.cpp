#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "light_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cLightPrivate::set_type(LightType t)
	{
		type = t;
	}

	void cLightPrivate::set_color(const vec3& c)
	{
		color = c;
	}

	void cLightPrivate::set_cast_shadow(bool v)
	{
		cast_shadow = v;
	}

	mat4 cLightPrivate::get_shadow_mat(uint idx) const
	{
		if (s_renderer)
			return s_renderer->get_shaodw_mat(id, idx);
		return mat4(1);
	}

	void cLightPrivate::draw(sRendererPtr s_renderer, bool first, bool)
	{
		if (first)
			id = s_renderer->add_light(node->transform, type, color, cast_shadow);
	}

	void cLightPrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		assert(node);
	}

	void cLightPrivate::on_removed()
	{
		node = nullptr;
	}

	void cLightPrivate::on_entered_world()
	{
		s_renderer = entity->world->get_system_t<sRendererPrivate>();
		assert(s_renderer);
	}

	void cLightPrivate::on_left_world()
	{
		s_renderer = nullptr;
	}

	cLight* cLight::create()
	{
		return new cLightPrivate();
	}
}
