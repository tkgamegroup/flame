#include <flame/graphics/canvas.h>
#include "../entity_private.h"
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

	void cLightPrivate::set_cast_shadow(bool v)
	{
		cast_shadow = v;
	}

	void cLightPrivate::draw(graphics::Canvas* canvas)
	{
		node->update_transform();
		canvas->add_light(type, type == graphics::LightPoint ? mat3(node->g_pos, vec3(0.f), vec3(0.f)) : node->g_rot, color, cast_shadow);
	}

	void cLightPrivate::on_added()
	{
		node = entity->get_component_t<cNodePrivate>();
		fassert(node);

		drawer = node->add_drawer([](Capture& c, graphics::Canvas* canvas) {
			auto thiz = c.thiz<cLightPrivate>();
			thiz->draw(canvas);
		}, Capture().set_thiz(this));
		node->mark_drawing_dirty();
	}

	void cLightPrivate::on_removed()
	{
		node->remove_drawer(drawer);
		node = nullptr;
	}

	cLight* cLight::create()
	{
		return f_new<cLightPrivate>();
	}
}
