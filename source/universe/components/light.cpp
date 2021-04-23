#include "../entity_private.h"
#include "node_private.h"
#include "light_private.h"
#include "../systems/renderer_private.h"

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

	void cLightPrivate::set_cast_shadow(bool v)
	{
		cast_shadow = v;
	}

	void cLightPrivate::draw(sRendererPtr renderer)
	{
		renderer->add_light(node, color, cast_shadow);
	}

	void cLightPrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		fassert(node);

		drawer = node->add_drawer([](Capture& c, sRendererPtr renderer) {
			auto thiz = c.thiz<cLightPrivate>();
			thiz->draw(renderer);
		}, Capture().set_thiz(this));
		node->mark_drawing_dirty();
	}

	void cLightPrivate::on_removed()
	{
		node->remove_drawer(drawer);
		node = nullptr;
	}

	cLight* cLight::create(void* parms)
	{
		return f_new<cLightPrivate>();
	}
}
