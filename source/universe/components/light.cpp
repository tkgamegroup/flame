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
		canvas->add_light(type, Mat4f(Mat<3, 4, float>(node->global_axes, Vec3f(0.f)), Vec4f(node->global_pos, 1.f)), color, cast_shadow);
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
