#include <flame/graphics/canvas.h>
#include "node_private.h"
#include "light_private.h"

namespace flame
{
	void cLightPrivate::set_type(graphics::LightType t)
	{
		type = t;
	}

	void cLightPrivate::set_color(const Vec3f& c)
	{
		color = c;
	}

	void cLightPrivate::draw(graphics::Canvas* canvas)
	{
		node->update_transform();
		canvas->add_light(type, type == graphics::LightPoint ? Mat3f(node->global_pos, Vec3f(0.f), Vec3f(0.f)) : node->global_dirs, color, cast_shadow);
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
