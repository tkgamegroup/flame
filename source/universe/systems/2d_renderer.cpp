#include <flame/serialize.h>
#include <flame/graphics/device.h>
#include "../entity_private.h"
#include <flame/universe/world.h>
#include <flame/universe/systems/2d_renderer.h>
#include "../components/element_private.h"

#include "../renderpath/canvas/canvas.h"

#include <flame/reflect_macros.h>

namespace flame
{
	struct R(Serializer_Atlas, flame,)
	{
		RV(StringW, filename, n);

		FLAME_UNIVERSE_EXPORTS RF(Serializer_Atlas)()
		{
		}

		FLAME_UNIVERSE_EXPORTS RF(~Serializer_Atlas)()
		{
		}

		FLAME_UNIVERSE_EXPORTS Object* RF(create)(World* w)
		{
			auto a = graphics::Atlas::load(graphics::Device::default_one(), filename.v);
			auto renderer = w->get_system(s2DRenderer);
			if (renderer)
				renderer->canvas->add_atlas(a);
			return a;
		}

		FLAME_UNIVERSE_EXPORTS void RF(destroy)(Object* o)
		{
			auto a = (graphics::Atlas*)o;
			((graphics::Canvas*)a->canvas_)->set_image(a->canvas_slot_, nullptr);
			graphics::Atlas::destroy(a);
		}
	};

	struct R(Serializer_FontAtlas, flame,)
	{
		RV(graphics::FontDrawType, draw_type, n);
		RV(StringW, fonts, n);

		FLAME_UNIVERSE_EXPORTS RF(Serializer_FontAtlas)()
		{
			draw_type = graphics::FontDrawPixel;
		}

		FLAME_UNIVERSE_EXPORTS RF(~Serializer_FontAtlas)()
		{
		}

		FLAME_UNIVERSE_EXPORTS Object* RF(create)(World* w)
		{
			auto sp = SUW::split(fonts.str(), L';');
			std::vector<const wchar_t*> fonts(sp.size());
			for (auto i = 0; i < sp.size(); i++)
				fonts[i] = sp[i].c_str();
			auto f = graphics::FontAtlas::create(graphics::Device::default_one(), draw_type, fonts.size(), fonts.data());
			auto renderer = w->get_system(s2DRenderer);
			if (renderer)
				renderer->canvas->add_font(f);
			return f;
		}

		FLAME_UNIVERSE_EXPORTS void RF(destroy)(Object* o)
		{
			auto f = (graphics::FontAtlas*)o;
			((graphics::Canvas*)f->canvas_)->set_image(f->canvas_slot_, nullptr);
			graphics::FontAtlas::destroy(f);
		}
	};

	struct s2DRendererPrivate : s2DRenderer
	{
		s2DRendererPrivate(graphics::Canvas* _canvas)
		{
			canvas = _canvas;
		}

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

		void update(Entity* root) override
		{
			do_render((EntityPrivate*)root);
			canvas->scene->update();
		}
	};

	s2DRenderer* s2DRenderer::create(const wchar_t* canvas_filename, void* dst, uint dst_hash, void* cbs)
	{
		return new s2DRendererPrivate(graphics::Canvas::create(canvas_filename, dst, dst_hash, cbs));
	}

	void s2DRenderer::destroy(s2DRenderer* s)
	{
		delete (s2DRendererPrivate*)s;
	}
}
