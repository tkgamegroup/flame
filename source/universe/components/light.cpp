#include <flame/graphics/canvas.h>
#include "node_private.h"
#include "light_private.h"

namespace flame
{
	void cLightPrivate::set_type(graphics::LightType t)
	{
		type = t;
	}

	void cLightPrivate::set_color(const vec3& c)
	{
		color = c;
	}

	void cLightPrivate::draw(graphics::Canvas* canvas)
	{
		node->update_transform();
		canvas->add_light(type, type == graphics::LightPoint ? mat3(node->global_pos, vec3(0.f), vec3(0.f)) : node->global_dirs, color, cast_shadow);
	}

	void cLightPrivate::set_cast_shadow(bool v)
	{
		cast_shadow = v;
	}

	cLight* cLight::create()
	{
		return f_new<cLightPrivate>();
	}
}
