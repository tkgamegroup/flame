#include <flame/graphics/canvas.h>
#include <flame/universe/components/element.h>

namespace flame
{
	struct cElementPrivate : cElement
	{
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

		void on_entity_added_to_parent()
		{
			auto e = entity->parent();
			if (e)
				p_element = (cElementPrivate*)(e->find_component(cH("Element")));
			else
				p_element = nullptr;
		}

		void start()
		{
			auto e = entity->parent();
			if (e)
				p_element = (cElementPrivate*)(e->find_component(cH("Element")));
			if (p_element)
				canvas = p_element->canvas;
			assert(canvas);
		}

		void update()
		{
			if (!p_element)
			{
				global_pos = pos;
				global_scale = scale;
			}
			else
			{
				global_pos = p_element->global_pos + p_element->global_scale * pos;
				global_scale = p_element->global_scale * scale;
			}
			global_size = size * global_scale;

			if (clip_children || !p_element)
			{
				auto cp = global_pos + Vec2f(inner_padding[0], inner_padding[1]) * global_scale;
				auto cs = global_size - Vec2f(inner_padding_horizontal(), inner_padding_vertical()) * global_scale;
				scissor = Vec4f(cp, cp + cs);
			}
			else
			{
				scissor = p_element->scissor;
				canvas->set_scissor(scissor);
			}

			cliped = false;
			if (!p_element || rect_overlapping(p_element->scissor, Vec4f(global_pos, global_pos + global_size)))
			{
				auto r = roundness * global_scale;
				auto st = shadow_thickness * global_scale;

				if (st > 0.f)
				{
					std::vector<Vec2f> points;
					path_rect(points, global_pos - Vec2f(st * 0.5f), global_size + Vec2f(st), r);
					points.push_back(points[0]);
					canvas->stroke(points, Vec4c(0, 0, 0, 128), Vec4c(0), st);
				}
				if (alpha > 0.f)
				{
					std::vector<Vec2f> points;
					path_rect(points, global_pos, global_size, r);
					if (color.w() > 0)
						canvas->fill(points, alpha_mul(color, alpha));
					auto ft = frame_thickness * global_scale;
					if (ft > 0.f && frame_color.w() > 0)
					{
						points.push_back(points[0]);
						canvas->stroke(points, alpha_mul(frame_color, alpha), ft);
					}
				}
			}
			else
				cliped = true;
		}

		Component* copy()
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

	void cElement::on_entity_added_to_parent()
	{
		((cElementPrivate*)this)->on_entity_added_to_parent();
	}

	void cElement::start()
	{
		((cElementPrivate*)this)->start();
	}

	void cElement::update()
	{
		((cElementPrivate*)this)->update();
	}

	Component* cElement::copy()
	{
		return ((cElementPrivate*)this)->copy();
	}

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
