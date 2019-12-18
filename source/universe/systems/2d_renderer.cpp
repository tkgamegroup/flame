#include <flame/graphics/device.h>
#include "../entity_private.h"
#include "../universe_private.h"
#include <flame/universe/world.h>
#include <flame/universe/systems/2d_renderer.h>
#include "../components/element_private.h"

#include "../renderpath/canvas/canvas.h"

namespace flame
{
	struct Serializer_Atlas$
	{
		std::wstring filename$;

		FLAME_UNIVERSE_EXPORTS Serializer_Atlas$()
		{
		}

		FLAME_UNIVERSE_EXPORTS ~Serializer_Atlas$()
		{
		}

		FLAME_UNIVERSE_EXPORTS Object* create$(World* w)
		{
			auto a = graphics::Atlas::load(graphics::Device::default_one(), filename$);
			auto canvas = (graphics::Canvas*)w->find_object(cH("Canvas"), 0);
			if (canvas)
				canvas->add_atlas(a);
			return a;
		}

		FLAME_UNIVERSE_EXPORTS void destroy$(Object* o)
		{
			auto a = (graphics::Atlas*)o;
			((graphics::Canvas*)a->canvas_)->set_image(a->canvas_slot_, nullptr);
			graphics::Atlas::destroy(a);
		}
	};

	struct Serializer_FontAtlas$
	{
		graphics::FontDrawType$ draw_type$;
		std::wstring fonts$;

		FLAME_UNIVERSE_EXPORTS Serializer_FontAtlas$()
		{
			draw_type$ = graphics::FontDrawPixel;
		}

		FLAME_UNIVERSE_EXPORTS ~Serializer_FontAtlas$()
		{
		}

		FLAME_UNIVERSE_EXPORTS Object* create$(World* w)
		{
			auto fonts = string_split(fonts$, L';');
			auto f = graphics::FontAtlas::create(graphics::Device::default_one(), draw_type$, fonts);
			auto canvas = (graphics::Canvas*)w->find_object(cH("Canvas"), 0);
			if (canvas)
				canvas->add_font(f);
			return f;
		}

		FLAME_UNIVERSE_EXPORTS void destroy$(Object* o)
		{
			auto f = (graphics::FontAtlas*)o;
			((graphics::Canvas*)f->canvas_)->set_image(f->canvas_slot_, nullptr);
			graphics::FontAtlas::destroy(f);
		}
	};

	struct s2DRendererPrivate : s2DRenderer
	{
		graphics::Canvas* canvas;

		void do_render(EntityPrivate* e)
		{
			if (!e->global_visibility_)
				return;

			auto element = (cElementPrivate*)e->get_component(cElement);
			if (!element)
				return;

			const auto& scissor = canvas->scissor();
			auto r = rect(element->global_pos, element->global_size);
			element->cliped = !rect_overlapping(scissor, r);
			element->cliped_rect = element->cliped ? Vec4f(-1.f) : Vec4f(max(r.x(), scissor.x()), max(r.y(), scissor.y()), min(r.z(), scissor.z()), min(r.w(), scissor.w()));
			element->draw(canvas);

			if (element->clip_children)
			{
				auto last_scissor = canvas->scissor();
				auto scissor = Vec4f(element->global_pos, element->global_pos + element->global_size);
				scissor += Vec4f(element->inner_padding_[0], element->inner_padding_[1], -element->inner_padding_[2], -element->inner_padding_[3]) * element->global_scale;
				canvas->set_scissor(scissor);
				for (auto& c : e->children)
					do_render(c.get());
				canvas->set_scissor(last_scissor);
			}
			else
			{
				for (auto& c : e->children)
					do_render(c.get());
			}
		}

		void on_added() override
		{
			canvas = (graphics::Canvas*)world_->find_object(cH("Canvas"), 0);
		}

		void update(Entity* root) override
		{
			do_render((EntityPrivate*)root);
		}
	};

	s2DRenderer* s2DRenderer::create()
	{
		return new s2DRendererPrivate();
	}

	void s2DRenderer::destroy(s2DRenderer* s)
	{
		delete (s2DRendererPrivate*)s;
	}

	struct Serializer_sUIRenderer$
	{
		FLAME_UNIVERSE_EXPORTS System* create$(World* w)
		{
			return new s2DRendererPrivate();
		}
	};
}
