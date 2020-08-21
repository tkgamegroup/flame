#include <flame/graphics/canvas.h>
#include "../world_private.h"
#include "../components/element_private.h"
#include "element_renderer_private.h"

namespace flame
{
	void sElementRendererPrivate::do_render(EntityPrivate* e)
	{
		if (!e->global_visibility)
			return;

		auto element = (cElementPrivate*)e->get_component(cElement::type_hash);
		if (!element)
			return;

		element->scissor = canvas->get_scissor();
		element->update_transform();
		element->culled = !rect_overlapping(element->scissor, element->aabb);
		if (element->culled)
			return;

		for (auto d : element->before_drawers)
			d->draw(canvas);
		element->draw(canvas);
		if (element->clipping)
			canvas->set_scissor(Vec4f(element->points[4], element->points[6]));
		for (auto d : element->after_drawers)
			d->draw(canvas);
		for (auto& c : e->children)
			do_render(c.get());
		if (element->clipping)
			canvas->set_scissor(element->scissor);
	}

	void sElementRendererPrivate::on_added()
	{
		canvas = (graphics::Canvas*)((WorldPrivate*)world)->find_object("flame::graphics::Canvas");
	}

	void sElementRendererPrivate::update()
	{
		if (!dirty && !always_update)
			return;
		do_render(((WorldPrivate*)world)->root.get());
		dirty = false;
	}

	sElementRenderer* sElementRenderer::create()
	{
		return f_new<sElementRendererPrivate>();
	}
}
