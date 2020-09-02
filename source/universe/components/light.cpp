#include <flame/graphics/canvas.h>
#include "node_private.h"
#include "light_private.h"

namespace flame
{
	void cLightPrivate::draw(graphics::Canvas* canvas, cCamera* _camera)
	{

	}

	cLight* cLight::create()
	{
		return f_new<cLightPrivate>();
	}
}
