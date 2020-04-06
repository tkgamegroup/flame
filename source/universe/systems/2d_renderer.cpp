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
	struct Serializer_Atlas
	{
		RV(StringW, filename, n);

		Object* create(World* w)
		{
			auto a = graphics::Atlas::load(graphics::Device::default_one(), filename.v);
			auto renderer = w->get_system(s2DRenderer);
			if (renderer)
				renderer->canvas->add_atlas(a);
			return a;
		}

		void destroy(Object* o)
		{
			auto a = (graphics::Atlas*)o;
			((graphics::Canvas*)a->canvas_)->set_image(a->canvas_slot_, nullptr);
			graphics::Atlas::destroy(a);
		}
	};

	struct Serializer_FontAtlas
	{
		RV(graphics::FontDrawType, draw_type, n);
		RV(StringW, fonts, n);

		Object* create(World* w)
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

		void destroy(Object* o)
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
			canvas->set_draw([](void* c) {
				auto thiz = *(s2DRendererPrivate**)c;
				thiz->do_render((EntityPrivate*)thiz->world_->root());
			}, Mail::from_p(this));

			pending_update = false;
		}

		~s2DRendererPrivate()
		{
			BP::destroy(canvas->scene);
		}

		void do_render(EntityPrivate* e)
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
				auto scissor = Vec4f(element->global_pos, element->global_pos + element->global_size);
				scissor += Vec4f(element->inner_padding_[0], element->inner_padding_[1], -element->inner_padding_[2], -element->inner_padding_[3]) * element->global_scale;
				if (clip_flags == (ClipSelf | ClipChildren))
				{
					element->draw(canvas);
					canvas->set_scissor(scissor);
					element->cmds.call(canvas);
					for (auto& c : e->children)
						do_render(c.get());
					canvas->set_scissor(last_scissor);
				}
				else if (clip_flags == ClipSelf)
				{
					element->draw(canvas);
					canvas->set_scissor(scissor);
					element->cmds.call(canvas);
					canvas->set_scissor(last_scissor);
					for (auto& c : e->children)
						do_render(c.get());
				}
				else if (clip_flags == ClipChildren)
				{
					element->draw(canvas);
					element->cmds.call(canvas);
					canvas->set_scissor(scissor);
					for (auto& c : e->children)
						do_render(c.get());
					canvas->set_scissor(last_scissor);
				}
			}
			else
			{
				element->draw(canvas);
				element->cmds.call(canvas);
				for (auto& c : e->children)
					do_render(c.get());
			}
		}

		void update() override
		{
			if (!pending_update)
				return;
			pending_update = false;
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
