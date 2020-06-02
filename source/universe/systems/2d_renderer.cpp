#include <flame/serialize.h>
#include <flame/graphics/device.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/entity.h>
#include <flame/universe/world.h>
#include <flame/universe/systems/2d_renderer.h>
#include "../components/element_private.h"

namespace flame
{
	struct s2DRendererPrivate : s2DRenderer
	{
		s2DRendererPrivate(graphics::Canvas* _canvas)
		{
			canvas = _canvas;

			pending_update = false;
		}

		void do_render(Entity* e)
		{
			if (!e->global_visibility)
				return;

			auto element = (cElementPrivate*)e->get_component(cElement);
			if (!element)
				return;

			const auto& scissor = canvas->scissor();
			auto r = rect(element->global_pos, element->global_size);
			element->clipped = !rect_overlapping(scissor, r);
			element->clipped_rect = element->clipped ? Vec4f(-1.f) : Vec4f(max(r.x(), scissor.x()), max(r.y(), scissor.y()), min(r.z(), scissor.z()), min(r.w(), scissor.w()));

			auto clip_flags = element->clip_flags;
			if (clip_flags)
			{
				auto last_scissor = canvas->scissor();
				auto scissor = Vec4f(element->content_min(), element->content_max());
				if (clip_flags == (ClipSelf | ClipChildren))
				{
					element->draw(canvas);
					canvas->set_scissor(scissor);
					element->cmds.call(canvas);
					for (auto c : e->children)
						do_render(c);
					canvas->set_scissor(last_scissor);
				}
				else if (clip_flags == ClipSelf)
				{
					element->draw(canvas);
					canvas->set_scissor(scissor);
					element->cmds.call(canvas);
					canvas->set_scissor(last_scissor);
					for (auto c : e->children)
						do_render(c);
				}
				else if (clip_flags == ClipChildren)
				{
					element->draw(canvas);
					element->cmds.call(canvas);
					canvas->set_scissor(scissor);
					for (auto c : e->children)
						do_render(c);
					canvas->set_scissor(last_scissor);
				}
			}
			else
			{
				element->draw(canvas);
				element->cmds.call(canvas);
				for (auto c : e->children)
					do_render(c);
			}
		}

		void update() override
		{
			if (!pending_update)
				return;
			do_render(world_->root);
		}

		void after_update() override
		{
			pending_update = false;
		}
	};

	s2DRenderer* s2DRenderer::create(graphics::Canvas* canvas)
	{
		return new s2DRendererPrivate(canvas);
	}

	void s2DRenderer::destroy(s2DRenderer* s)
	{
		delete (s2DRendererPrivate*)s;
	}
}
