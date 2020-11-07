#include <flame/graphics/image.h>
#include <flame/graphics/canvas.h>
#include "../world_private.h"
#include "../components/element_private.h"
#include "../components/node_private.h"
#include "../components/custom_drawing_private.h"
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
					for (auto& d : element->drawers[0])
						d.second(d.first, canvas);
					element->draw(canvas);
					if (element->clipping)
						canvas->set_scissor(element->aabb);
					for (auto& d : element->drawers[1])
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
					d.second(d.first, canvas);
			}
		}

		{
			auto custom = e->get_component_t<cCustomDrawingPrivate>();
			if (custom)
			{
				for (auto& d : custom->drawers)
					d->call(canvas);
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
		{
			auto node = camera->node;
			node->update_transform();
			auto out = canvas->get_output(0);
			auto size = out ? Vec2f(out->get_image()->get_size()) : Vec2f(1.f);
			canvas->set_camera(camera->fovy, size.x() / size.y(), camera->near, camera->far, node->global_dirs, node->global_pos);
		}
		render(world->root.get(), false, !camera);
		dirty = false;
	}

	sRenderer* sRenderer::create()
	{
		return f_new<sRendererPrivate>();
	}
}
