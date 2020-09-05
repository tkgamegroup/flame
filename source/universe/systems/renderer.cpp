#include <flame/graphics/canvas.h>
#include "../world_private.h"
#include "../components/element_private.h"
#include "../components/node_private.h"
#include "../components/camera_private.h"
#include "renderer_private.h"

namespace flame
{
	void sRendererPrivate::render(EntityPrivate* e, bool element_culled, bool node_culled)
	{
		if (!e->global_visibility)
			return;

		auto last_scissor = canvas->get_scissor();

		if (!element_culled)
		{
			auto element = e->get_component_t<cElementPrivate>();
			if (element)
			{
				element->boundaries = last_scissor;
				element->update_transform();
				element->culled = !rect_overlapping(element->boundaries, element->aabb);
				if (element->culled)
					element_culled = true;
				else
				{
					for (auto& d : element->underlayer_drawers)
						d.second(d.first, canvas);
					element->draw(canvas);
					if (element->clipping)
						canvas->set_scissor(Vec4f(element->points[4], element->points[6]));
					for (auto& d : element->drawers)
						d.second(d.first, canvas);
				}
			}
		}
		if (!node_culled)
		{
			auto node = e->get_component_t<cNodePrivate>();
			if (node)
			{
				node->update_transform();
				for (auto& d : node->drawers)
					d.second(d.first, canvas, camera);
			}
		}
		for (auto& c : e->children)
			render(c.get(), element_culled, node_culled);

		canvas->set_scissor(last_scissor);
	}

	void sRendererPrivate::on_added()
	{
		canvas = (graphics::Canvas*)world->find_object("flame::graphics::Canvas");
	}

	void sRendererPrivate::update()
	{
		if (!dirty && !always_update)
			return;
		if (camera)
			camera->update_matrix();
		render(world->root.get(), false, !camera);
		dirty = false;
	}

	sRenderer* sRenderer::create()
	{
		return f_new<sRendererPrivate>();
	}
}
