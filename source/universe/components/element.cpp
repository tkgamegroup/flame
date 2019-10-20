#include "../entity_private.h"
#include <flame/universe/components/element.h>

#include "../renderpath/canvas_make_cmd/canvas.h"

namespace flame
{
	struct cElementPrivate : cElement
	{
		void boardcast_canvas(graphics::Canvas* canvas, EntityPrivate* e)
		{
			auto c = e->get_component(Element);
			if (c)
				c->canvas = canvas;
			for (auto& c : e->children)
				boardcast_canvas(canvas, c.get());
		}

		cElementPrivate(graphics::Canvas* _canvas)
		{
			pos = 0.f;
			scale = 1.f;
			size = 0.f;
			inner_padding = Vec4f(0.f);
			alpha = 1.f;
			roundness = Vec4f(0.f);
			frame_thickness = 0.f;
			color = Vec4c(0);
			frame_color = Vec4c(255);
			shadow_thickness = 0.f;
			clip_children = false;

			p_element = nullptr;
			canvas = _canvas;
			global_pos = 0.f;
			global_scale = 0.f;
			global_size = 0.f;
		}

		void on_added() override
		{
			auto p = entity->parent();
			if (p)
			{
				p_element = p->get_component(Element);
				if (p_element)
					canvas = p_element->canvas;
			}
		}

		void on_child_component_added(Component* _c) override
		{
			if (_c->type_hash == cH("Element"))
			{
				auto c = (cElement*)_c;
				c->p_element = this;
				if (!c->canvas && canvas)
					boardcast_canvas(canvas, (EntityPrivate*)c->entity);
			}
		}

		Component* copy() override
		{
			auto copy = new cElementPrivate(canvas);

			copy->pos = pos;
			copy->scale = scale;
			copy->size = size;
			copy->inner_padding = inner_padding;
			copy->alpha = alpha;
			copy->roundness = roundness;
			copy->frame_thickness = frame_thickness;
			copy->color = color;
			copy->frame_color = frame_color;
			copy->shadow_thickness = shadow_thickness;
			copy->clip_children = clip_children;

			return copy;
		}
	};

	cElement* cElement::create(graphics::Canvas* canvas)
	{
		return new cElementPrivate(canvas);
	}

	struct ComponentElement$
	{
		Vec2f pos$;
		float scale$;
		Vec2f size$;
		Vec4f inner_padding$;
		float alpha$;
		Vec4f roundness$;
		float frame_thickness$;
		Vec4c color$;
		Vec4c frame_color$;
		float shadow_thickness$;
		bool clip_children$;

		FLAME_UNIVERSE_EXPORTS ComponentElement$()
		{
			pos$ = 0.f;
			scale$ = 1.f;
			size$ = 0.f;
			inner_padding$ = Vec4f(0.f);
			alpha$ = 1.f;
			roundness$ = Vec4f(0.f);
			frame_thickness$ = 0.f;
			color$ = Vec4c(0);
			frame_color$ = Vec4c(255);
			shadow_thickness$ = 0.f;
			clip_children$ = false;
		}

		FLAME_UNIVERSE_EXPORTS Component* create$()
		{
			auto c = new cElementPrivate(nullptr);

			c->pos = pos$;
			c->scale = scale$;
			c->size = size$;
			c->inner_padding = inner_padding$;
			c->alpha = alpha$;
			c->inner_padding = inner_padding$;
			c->frame_thickness = frame_thickness$;
			c->color = color$;
			c->frame_color = frame_color$;
			c->shadow_thickness = shadow_thickness$;
			c->clip_children = clip_children$;

			return c;
		}

		FLAME_UNIVERSE_EXPORTS void serialize$(Component* _c, int offset)
		{
			auto c = (cElement*)_c;

			if (offset == -1)
			{
				pos$ = c->pos;
				scale$ = c->scale;
				size$ = c->size;
				inner_padding$ = c->inner_padding;
				alpha$ = c->alpha;
				roundness$ = c->roundness;
				frame_thickness$ = c->frame_thickness;
				color$ = c->color;
				frame_color$ = c->frame_color;
				shadow_thickness$ = c->shadow_thickness;
				clip_children$ = c->clip_children;
			}
			else
			{
				switch (offset)
				{
				case offsetof(ComponentElement$, pos$):
					pos$ = c->pos;
					break;
				case offsetof(ComponentElement$, scale$):
					scale$ = c->scale;
					break;
				case offsetof(ComponentElement$, size$):
					size$ = c->size;
					break;
				case offsetof(ComponentElement$, inner_padding$):
					inner_padding$ = c->inner_padding;
					break;
				case offsetof(ComponentElement$, alpha$):
					alpha$ = c->alpha;
					break;
				case offsetof(ComponentElement$, roundness$):
					roundness$ = c->roundness;
					break;
				case offsetof(ComponentElement$, frame_thickness$):
					frame_thickness$ = c->frame_thickness;
					break;
				case offsetof(ComponentElement$, color$):
					color$ = c->color;
					break;
				case offsetof(ComponentElement$, frame_color$):
					frame_color$ = c->frame_color;
					break;
				case offsetof(ComponentElement$, shadow_thickness$):
					shadow_thickness$ = c->shadow_thickness;
					break;
				case offsetof(ComponentElement$, clip_children$):
					clip_children$ = c->clip_children;
					break;
				}
			}
		}

		FLAME_UNIVERSE_EXPORTS void unserialize$(Component* _c, int offset)
		{
			auto c = (cElement*)_c;

			if (offset == -1)
			{
				c->pos = pos$;
				c->scale = scale$;
				c->size = size$;
				c->inner_padding = inner_padding$;
				c->alpha = alpha$;
				c->roundness = roundness$;
				c->frame_thickness = frame_thickness$;
				c->color = color$;
				c->frame_color = frame_color$;
				c->shadow_thickness = shadow_thickness$;
				c->clip_children = clip_children$;
			}
			else
			{
				switch (offset)
				{
				case offsetof(ComponentElement$, pos$):
					c->pos = pos$;
					break;
				case offsetof(ComponentElement$, scale$):
					c->scale = scale$;
					break;
				case offsetof(ComponentElement$, size$):
					c->size = size$;
					break;
				case offsetof(ComponentElement$, inner_padding$):
					c->inner_padding = inner_padding$;
					break;
				case offsetof(ComponentElement$, alpha$):
					c->alpha = alpha$;
					break;
				case offsetof(ComponentElement$, roundness$):
					c->roundness = roundness$;
					break;
				case offsetof(ComponentElement$, frame_thickness$):
					c->frame_thickness = frame_thickness$;
					break;
				case offsetof(ComponentElement$, color$):
					c->color = color$;
					break;
				case offsetof(ComponentElement$, frame_color$):
					c->frame_color = frame_color$;
					break;
				case offsetof(ComponentElement$, shadow_thickness$):
					c->shadow_thickness = shadow_thickness$;
					break;
				case offsetof(ComponentElement$, clip_children$):
					c->clip_children = clip_children$;
					break;
				}
			}
		}
	};
}
