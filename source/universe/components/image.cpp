#include <flame/universe/world.h>
#include <flame/universe/components/element.h>
#include "image_private.h"

#include "../renderpath/canvas_make_cmd/canvas.h"

namespace flame
{
	cImagePrivate::cImagePrivate()
	{
		element = nullptr;

		id = 0;
		uv0 = Vec2f(0.f);
		uv1 = Vec2f(1.f);
		color = Vec4c(255);
	}

	void cImagePrivate::on_component_added(Component* c)
	{
		if (c->name_hash == cH("cElement"))
			element = (cElement*)c;
	}

	void cImagePrivate::draw(graphics::Canvas* canvas)
	{
		auto padding = element->inner_padding_ * element->global_scale;
		auto pos = element->global_pos + Vec2f(padding[0], padding[1]);
		auto size = element->global_size - Vec2f(padding[0] + padding[2], padding[1] + padding[3]);
		canvas->add_image(pos, size, id, uv0, uv1, alpha_mul(color, element->alpha));
	}

	cImage* cImage::create()
	{
		return new cImagePrivate();
	}

	struct Serializer_cImage$
	{
		uint atlas_id$;
		uint region_id$;
		Vec2f uv0$;
		Vec2f uv1$;
		Vec4c color$;

		FLAME_UNIVERSE_EXPORTS Serializer_cImage$()
		{
			atlas_id$ = 0;
			region_id$ = 0;
			uv0$ = Vec2f(0.f);
			uv1$ = Vec2f(1.f);
			color$ = Vec4c(255);
		}

		FLAME_UNIVERSE_EXPORTS ~Serializer_cImage$()
		{
		}

		FLAME_UNIVERSE_EXPORTS Component* create$(World* w)
		{
			auto c = new cImagePrivate();

			auto atlas = (graphics::Atlas*)w->find_object(cH("Atlas"), atlas_id$);
			c->id = (atlas->canvas_slot_ << 16) + atlas->find_region(region_id$);
			c->uv0 = uv0$;
			c->uv1 = uv1$;
			c->color = color$;

			return c;
		}

		FLAME_UNIVERSE_EXPORTS void serialize$(Component* _c, int offset)
		{
			auto c = (cImage*)_c;
			auto w = c->entity->world_;

			if (offset == -1)
			{
				auto atlas = ((graphics::Canvas*)w->find_object(cH("Canvas"), 0))->get_atlas(c->id >> 16);
				atlas_id$ = atlas->id;
				region_id$ = atlas->regions()[c->id & 0xffff].id;
				color$ = c->color;
				uv0$ = c->uv0;
				uv1$ = c->uv1;
			}
			else
			{
				switch (offset)
				{
				case offsetof(Serializer_cImage$, color$):
					color$ = c->color;
					break;
				case offsetof(Serializer_cImage$, uv0$):
					uv0$ = c->uv0;
					break;
				case offsetof(Serializer_cImage$, uv1$):
					uv1$ = c->uv1;
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
				auto atlas = (graphics::Atlas*)w->find_object(cH("Atlas"), atlas_id$);
				c->id = (atlas->canvas_slot_ << 16) + atlas->find_region(region_id$);
				c->uv0 = uv0$;
				c->uv1 = uv1$;
				c->color = color$;
			}
			else
			{
				switch (offset)
				{
				case offsetof(Serializer_cImage$, color$):
					c->color = color$;
					break;
				case offsetof(Serializer_cImage$, uv0$):
					c->uv0 = uv0$;
					break;
				case offsetof(Serializer_cImage$, uv1$):
					c->uv1 = uv1$;
					break;
				}
			}
		}
	};
}
