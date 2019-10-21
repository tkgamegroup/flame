#include "../entity_private.h"
#include <flame/universe/systems/ui_renderer.h>
#include "../components/element_private.h"
#include "../components/text_private.h"
#include <flame/universe/components/aligner.h>

#include "../renderpath/canvas_make_cmd/canvas.h"

namespace flame
{
	struct sUIRendererPrivate : sUIRenderer
	{
		graphics::Canvas* canvas;

		sUIRendererPrivate(graphics::Canvas* canvas) :
			canvas(canvas)
		{
		}

		void draw_element_and_its_children(cElementPrivate* c_e)
		{
			auto e = (EntityPrivate*)c_e->entity;

			const auto& scissor = canvas->scissor();
			auto rect = Vec4f(c_e->global_pos, c_e->global_pos + c_e->global_size);
			c_e->cliped = !rect_overlapping(scissor, rect);
			if (!c_e->cliped)
			{
				c_e->cliped_rect = Vec4f(max(rect.x(), scissor.x()), max(rect.y(), scissor.y()), min(rect.z(), scissor.z()), min(rect.w(), scissor.w()));

				c_e->draw(canvas);

				auto c_t = (cTextPrivate*)e->get_component(Text);
				if (c_t)
					c_t->draw(canvas);
			}
			else
				c_e->cliped_rect = Vec4f(-1.f);

			for (auto& c : e->children)
				do_render(c.get());
		}

		void do_render(Entity* e)
		{
			if (!e->global_visibility_)
				return;

			auto element = (cElementPrivate*)e->get_component(Element);
			if (!element)
				return;

			if (element->clip_children)
			{
				auto last_scissor = canvas->scissor();
				auto scissor = Vec4f(element->global_pos, element->global_pos + element->global_size);
				scissor += Vec4f(element->inner_padding[0], element->inner_padding[1], element->inner_padding_horizontal(), element->inner_padding_vertical()) * element->global_scale;
				canvas->set_scissor(scissor);
				draw_element_and_its_children(element);
				canvas->set_scissor(last_scissor);
			}
			else
				draw_element_and_its_children(element);
		}

		void update(Entity* root) override
		{
			do_render(root);
		}
	};

	sUIRenderer* sUIRenderer::create(graphics::Canvas* canvas)
	{
		return new sUIRendererPrivate(canvas);
	}

	void sUIRenderer::destroy(sUIRenderer* s)
	{
		delete (sUIRendererPrivate*)s;
	}
}
