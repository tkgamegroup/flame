#include <flame/universe/world.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/image.h>

#include "../renderpath/canvas/canvas.h"

#include <flame/reflect_macros.h>

namespace flame
{
	struct cImagePrivate : cImage
	{
		void* draw_cmd;

		cImagePrivate()
		{
			element = nullptr;

			id = 0;
			uv0 = 0.f;
			uv1 = 1.f;
			color = 255;

			draw_cmd = nullptr;
		}

		~cImagePrivate()
		{
			if (!entity->dying_)
				element->cmds.remove(draw_cmd);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cElement"))
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
				canvas->add_image(pos, size, id, uv0, uv1, color.new_proply<3>(element->alpha_));
			}
		}
	};

	cImage* cImage::create()
	{
		return new cImagePrivate();
	}

	struct R(Serializer_cImage, flame,)
	{
		RV(ulonglong, id, n);
		RV(Vec2f, pos_offset, n);
		RV(Vec2f, size_offset, n);
		RV(Vec2f, uv0, n);
		RV(Vec2f, uv1, n);
		RV(Vec4c, color, n);

		FLAME_UNIVERSE_EXPORTS RF(Serializer_cImage)()
		{
			id = 0;
			pos_offset = 0.f;
			size_offset = 0.f;
			uv0 = 0.f;
			uv1 = 1.f;
			color = 255;
		}

		FLAME_UNIVERSE_EXPORTS RF(~Serializer_cImage)()
		{
		}

		FLAME_UNIVERSE_EXPORTS Component* RF(create)(World* w)
		{
			auto c = new cImagePrivate();

			auto atlas = (graphics::Atlas*)w->find_object(FLAME_CHASH("Atlas"), id >> 32);
			c->id = (atlas->canvas_slot_ << 16) + atlas->find_tile(id & 0xffffffff);
			c->uv0 = uv0;
			c->uv1 = uv1;
			c->color = color;

			return c;
		}

		FLAME_UNIVERSE_EXPORTS void RF(serialize)(Component* _c, int offset)
		{
			auto c = (cImagePrivate*)_c;
			auto w = c->entity->world_;

			if (offset == -1)
			{
				auto atlas = ((graphics::Canvas*)w->find_object(FLAME_CHASH("Canvas"), 0))->get_atlas(c->id >> 16);
				id = (atlas->id << 32) + atlas->tile(c->id & 0xffff).id;
				uv0 = c->uv0;
				uv1 = c->uv1;
				color = c->color;
			}
			else
			{
				switch (offset)
				{
				case offsetof(Serializer_cImage, id):
				{
					auto atlas = ((graphics::Canvas*)w->find_object(FLAME_CHASH("Canvas"), 0))->get_atlas(c->id >> 16);
					id = (atlas->id << 32) + atlas->tile(c->id & 0xffff).id;
				}
					break;
				case offsetof(Serializer_cImage, uv0):
					uv0 = c->uv0;
					break;
				case offsetof(Serializer_cImage, uv1):
					uv1 = c->uv1;
					break;
				case offsetof(Serializer_cImage, color):
					color = c->color;
					break;
				}
			}
		}

		FLAME_UNIVERSE_EXPORTS void  RF(unserialize)(Component* _c, int offset)
		{
			auto c = (cImagePrivate*)_c;
			auto w = c->entity->world_;

			if (offset == -1)
			{
				auto atlas = (graphics::Atlas*)w->find_object(FLAME_CHASH("Atlas"), id >> 32);
				c->id = (atlas->canvas_slot_ << 16) + atlas->find_tile(id & 0xffffffff);
				c->uv0 = uv0;
				c->uv1 = uv1;
				c->color = color;
			}
			else
			{
				switch (offset)
				{
				case offsetof(Serializer_cImage, id):
				{
					auto atlas = (graphics::Atlas*)w->find_object(FLAME_CHASH("Atlas"), id >> 32);
					c->id = (atlas->canvas_slot_ << 16) + atlas->find_tile(id & 0xffffffff);
				}
					break;
				case offsetof(Serializer_cImage, uv0):
					c->uv0 = uv0;
					break;
				case offsetof(Serializer_cImage, uv1):
					c->uv1 = uv1;
					break;
				case offsetof(Serializer_cImage, color):
					c->color = color;
					break;
				}
			}
		}
	};
}
