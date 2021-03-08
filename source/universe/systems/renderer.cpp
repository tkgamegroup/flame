#include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/canvas.h>
#include "../world_private.h"
#include "../components/element_private.h"
#include "../components/node_private.h"
#include "../components/camera_private.h"
#include "renderer_private.h"

namespace flame
{
	sRendererPrivate::sRendererPrivate(sRendererParms* _parms)
	{
		sRendererParms parms;
		if (_parms)
			parms = *_parms;

		hdr = parms.hdr;
	}

	void sRendererPrivate::set_targets()
	{
		fb_targets.clear();
		auto count = swapchain->get_images_count();
		fb_targets.resize(count);
		for (auto i = 0; i < count; i++)
		{
			auto v = swapchain->get_image(i)->get_view();

		}
	}

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
				element->parent_scissor = last_scissor;
				element->update_transform();
				element_culled = !last_scissor.overlapping(element->aabb);
				if (element->culled != element_culled)
				{
					element->culled = element_culled;
					e->component_data_changed(element, S<"culled"_h>);
				}
				if (!element_culled)
				{
					for (auto& d : element->drawers[0])
						d->call(canvas);
					element->draw(canvas);
					if (element->clipping)
						canvas->set_scissor(element->aabb);
					for (auto& d : element->drawers[1])
						d->call(canvas);
				}
				if (last_element != element)
				{
					last_element = element;
					last_element_changed = true;
				}
			}
		}
		if (!node_culled)
		{
			auto node = e->get_component_t<cNodePrivate>();
			if (node)
			{
				node->update_transform();
				if (last_element_changed)
					canvas->set_viewport(last_element->aabb);
				for (auto& d : node->drawers)
					d->call(canvas);
			}
		}

		for (auto& c : e->children)
			render(c.get(), element_culled, node_culled);

		canvas->set_scissor(last_scissor);
	}

	void sRendererPrivate::on_added()
	{
		window = (Window*)world->find_object("flame::Window");
		window->add_resize_listener([](Capture& c, const uvec2&) {
			c.thiz<sRendererPrivate>()->set_targets();
		}, Capture().set_thiz(this));
		swapchain;

		device = graphics::Device::get_default();
		swapchain = (graphics::Swapchain*)world->find_object("flame::graphics::Swapchain");

		set_targets();

		canvas = (graphics::Canvas*)world->find_object("flame::graphics::Canvas");
	}

	void sRendererPrivate::update()
	{
		if (!dirty && !always_update)
			return;
		last_element = nullptr;
		last_element_changed = false;
		render(world->root.get(), false, !camera);
		dirty = false;
	}

	sRenderer* sRenderer::create(void* parms)
	{
		return f_new<sRendererPrivate>((sRendererParms*)parms);
	}
}
