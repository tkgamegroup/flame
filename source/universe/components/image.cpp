#include <flame/universe/world.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/image.h>

#include "../renderpath/canvas_make_cmd/canvas.h"

namespace flame
{
	struct cImagePrivate : cImage
	{
		void* draw_cmd;

		cImagePrivate()
		{
			element = nullptr;

			id = 0;
			uv0 = Vec2f(0.f);
			uv1 = Vec2f(1.f);
			color = Vec4c(255);
			repeat = false;

			draw_cmd = nullptr;
		}

		~cImagePrivate()
		{
			if (!entity->dying_)
				element->cmds.remove(draw_cmd);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == cH("cElement"))
			{
				element = (cElement*)c;
				draw_cmd = element->cmds.add([](void* c, graphics::Canvas* canvas) {
					(*(cImagePrivate**)c)->draw(canvas);
				}, new_mail_p(this));
			}
		}

		void draw(graphics::Canvas* canvas)
		{
			if (!element->cliped)
			{
				auto padding = element->inner_padding_ * element->global_scale;
				auto pos = element->global_pos + Vec2f(padding[0], padding[1]);
				auto size = element->global_size - Vec2f(padding[0] + padding[2], padding[1] + padding[3]);
				canvas->add_image(pos, size, id, uv0, uv1, alpha_mul(color, element->alpha), repeat);
			}
		}
	};

	cImage* cImage::create()
	{
		return new cImagePrivate();
	}

	struct Serializer_cImage$
	{
		ulonglong id$;
		Vec2f uv0$;
		Vec2f uv1$;
		Vec4c color$;
		bool repeat$;

		FLAME_UNIVERSE_EXPORTS Serializer_cImage$()
		{
			id$ = 0;
			uv0$ = Vec2f(0.f);
			uv1$ = Vec2f(1.f);
			color$ = Vec4c(255);
			repeat$ = false;
		}

		FLAME_UNIVERSE_EXPORTS ~Serializer_cImage$()
		{
		}

		FLAME_UNIVERSE_EXPORTS Component* create$(World* w)
		{
			auto c = new cImagePrivate();

			auto atlas = (graphics::Atlas*)w->find_object(cH("Atlas"), id$ >> 32);
			c->id = (atlas->canvas_slot_ << 16) + atlas->find_region(id$ & 0xffffffff);
			c->uv0 = uv0$;
			c->uv1 = uv1$;
			c->color = color$;
			c->repeat = repeat$;

			return c;
		}

		FLAME_UNIVERSE_EXPORTS void serialize$(Component* _c, int offset)
		{
			auto c = (cImage*)_c;
			auto w = c->entity->world_;

			if (offset == -1)
			{
				auto atlas = ((graphics::Canvas*)w->find_object(cH("Canvas"), 0))->get_atlas(c->id >> 16);
				id$ = (atlas->id << 32) + atlas->regions()[c->id & 0xffff].id;
				color$ = c->color;
				uv0$ = c->uv0;
				uv1$ = c->uv1;
				repeat$ = c->repeat;
			}
			else
			{
				switch (offset)
				{
				case offsetof(Serializer_cImage$, id$):
				{
					auto atlas = ((graphics::Canvas*)w->find_object(cH("Canvas"), 0))->get_atlas(c->id >> 16);
					id$ = (atlas->id << 32) + atlas->regions()[c->id & 0xffff].id;
				}
					break;
				case offsetof(Serializer_cImage$, color$):
					color$ = c->color;
					break;
				case offsetof(Serializer_cImage$, uv0$):
					uv0$ = c->uv0;
					break;
				case offsetof(Serializer_cImage$, uv1$):
					uv1$ = c->uv1;
					break;
				case offsetof(Serializer_cImage$, repeat$):
					repeat$ = c->repeat;
					break;
				}
			}
		}

		FLAME_UNIVERSE_EXPORTS void unserialize$(Component* _c, int offset)
		{
			auto c = (cImage*)_c;
			auto w = c->entity->world_;

			if (offset == -1)
			{
				auto atlas = (graphics::Atlas*)w->find_object(cH("Atlas"), id$ >> 32);
				c->id = (atlas->canvas_slot_ << 16) + atlas->find_region(id$ & 0xffffffff);
				c->uv0 = uv0$;
				c->uv1 = uv1$;
				c->color = color$;
				c->repeat = repeat$;
			}
			else
			{
				switch (offset)
				{
				case offsetof(Serializer_cImage$, id$):
				{
					auto atlas = (graphics::Atlas*)w->find_object(cH("Atlas"), id$ >> 32);
					c->id = (atlas->canvas_slot_ << 16) + atlas->find_region(id$ & 0xffffffff);
				}
					break;
				case offsetof(Serializer_cImage$, color$):
					c->color = color$;
					break;
				case offsetof(Serializer_cImage$, uv0$):
					c->uv0 = uv0$;
					break;
				case offsetof(Serializer_cImage$, uv1$):
					c->uv1 = uv1$;
					break;
				case offsetof(Serializer_cImage$, repeat$):
					c->repeat = repeat$;
					break;
				}
			}
		}
	};
}
