//#include <flame/serialize.h>
//#include <flame/graphics/device.h>
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

		//auto scissor = canvas->get_scissor();
		//auto r = rect(element->global_pos, element->global_pos + element->global_size);
		//element->clipped = !rect_overlapping(scissor, r);
		//element->clipped_rect = element->clipped ? Vec4f(-1.f) : Vec4f(max(r.x(), scissor.x()), max(r.y(), scissor.y()), min(r.z(), scissor.z()), min(r.w(), scissor.w()));

		//auto clip_flags = element->clip_flags;
		//if (clip_flags)
		//{
		//	auto last_scissor = canvas->get_scissor();
		//	auto scissor = Vec4f(element->content_min(), element->content_max());
		//	if (clip_flags == (ClipSelf | ClipChildren))
		//	{
		//		element->draw(canvas);
		//		canvas->set_scissor(scissor);
		//		element->cmds.call(canvas);
		//		for (auto c : e->children)
		//			do_render(c);
		//		canvas->set_scissor(last_scissor);
		//	}
		//	else if (clip_flags == ClipSelf)
		//	{
		//		element->draw(canvas);
		//		canvas->set_scissor(scissor);
		//		element->cmds.call(canvas);
		//		canvas->set_scissor(last_scissor);
		//		for (auto c : e->children)
		//			do_render(c);
		//	}
		//	else if (clip_flags == ClipChildren)
		//	{
		//		element->draw(canvas);
		//		element->cmds.call(canvas);
		//		canvas->set_scissor(scissor);
		//		for (auto c : e->children)
		//			do_render(c);
		//		canvas->set_scissor(last_scissor);
		//	}
		//}
		//else
		//{
			element->draw(canvas);
			//element->cmds.call(canvas);
			for (auto& c : e->children)
				do_render(c.get());
		//}
	}

	void sElementRendererPrivate::on_added()
	{
		canvas = (graphics::Canvas*)((WorldPrivate*)world)->find_object("Canvas");
	}

	void sElementRendererPrivate::update()
	{
		if (!dirty)
			return;
		do_render(((WorldPrivate*)world)->root.get());
		dirty = false;
	}

	sElementRenderer* sElementRenderer::create()
	{
		return f_new<sElementRendererPrivate>();
	}
}
