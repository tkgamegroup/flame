#include <flame/graphics/canvas.h>
#include "node_private.h"
#include "light_private.h"

namespace flame
{
	void cLightPrivate::set_type(graphics::LightType t)
	{

	}

	void cLightPrivate::set_color(const Vec3f& c)
	{
		color = c;
	}

	void cLightPrivate::draw(graphics::Canvas* canvas, cCamera* _camera)
	{
		node->update_transform();
		canvas->add_light(type, color, Vec3f(node->transform[3]));
	}

	cLight* cLight::create()
	{
		return f_new<cLightPrivate>();
	}
}
