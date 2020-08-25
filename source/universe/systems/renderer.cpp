#include <flame/graphics/canvas.h>
#include "../world_private.h"
#include "../components/element_private.h"
#include "renderer_private.h"

namespace flame
{
	void sRendererPrivate::render(EntityPrivate* e, bool element_culled, bool node_culled)
	{
		if (!e->global_visibility)
			return;

		if (!element_culled)
		{
			auto element = e->get_component_t<cElementPrivate>();
			if (element)
			{
				element->scissor = canvas->get_scissor();
				element->update_transform();
				element->culled = !rect_overlapping(element->scissor, element->aabb);
				if (element->culled)
					return;

				for (auto& d : element->underlayer_drawers)
					d.second(d.first, canvas);
				element->draw(canvas);
				if (element->clipping)
					canvas->set_scissor(Vec4f(element->points[4], element->points[6]));
				for (auto& d : element->drawers)
					d.second(d.first, canvas);
				for (auto& c : e->children)
					render(c.get());
				if (element->clipping)
					canvas->set_scissor(element->scissor);
			}
		}
	}

	void sRendererPrivate::on_added()
	{
		canvas = (graphics::Canvas*)world->find_object("flame::graphics::Canvas");
	}

	void sRendererPrivate::update()
	{
		if (!dirty && !always_update)
			return;
		render(world->root.get());
		dirty = false;
	}

	sRenderer* sRenderer::create()
	{
		return f_new<sRendererPrivate>();
	}
}
