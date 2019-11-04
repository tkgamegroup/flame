#include <flame/graphics/device.h>
#include "../entity_private.h"
#include "../universe_private.h"
#include <flame/universe/world.h>
#include <flame/universe/systems/ui_renderer.h>
#include "../components/element_private.h"
#include "../components/text_private.h"
#include "../components/image_private.h"
#include <flame/universe/components/custom_draw.h>
#include <flame/universe/components/aligner.h>

#include "../renderpath/canvas_make_cmd/canvas.h"

namespace flame
{
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
			auto world_dir = std::filesystem::path(w->filename()).parent_path().wstring() + L"/";
			for (auto& f : fonts)
				f = world_dir + f;
			return graphics::FontAtlas::create(graphics::Device::default_one(), draw_type$, fonts);
		}
	};

	struct sUIRendererPrivate : sUIRenderer
	{
		graphics::Canvas* canvas;

		void do_render(EntityPrivate* e)
		{
			if (!e->global_visibility_)
				return;

			auto element = (cElementPrivate*)e->get_component(Element);
			if (!element)
				return;

			const auto& scissor = canvas->scissor();
			auto rect = Vec4f(element->global_pos, element->global_pos + element->global_size);
			element->cliped = !rect_overlapping(scissor, rect);
			if (!element->cliped)
			{
				element->cliped_rect = Vec4f(max(rect.x(), scissor.x()), max(rect.y(), scissor.y()), min(rect.z(), scissor.z()), min(rect.w(), scissor.w()));

				element->draw(canvas);

				auto text = (cTextPrivate*)e->get_component(Text);
				if (text)
					text->draw(canvas);

				auto image = (cImagePrivate*)e->get_component(Image);
				if (image)
					image->draw(canvas);

				auto cd = e->get_component(CustomDraw);
				if (cd)
				{
					auto& cmds = ((ListenerHub*)cd->cmds.hub)->listeners;
					for (auto& l : cmds)
						((void(*)(void*, graphics::Canvas*))l->function)(l->capture.p, canvas);
				}
			}
			else
				element->cliped_rect = Vec4f(-1.f);

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
			canvas = (graphics::Canvas*)world_->find_object(cH("Canvas"), "");
		}

		void update(Entity* root) override
		{
			do_render((EntityPrivate*)root);
		}
	};

	sUIRenderer* sUIRenderer::create()
	{
		return new sUIRendererPrivate();
	}

	void sUIRenderer::destroy(sUIRenderer* s)
	{
		delete (sUIRendererPrivate*)s;
	}
}
