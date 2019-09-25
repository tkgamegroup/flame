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
			round_radius = 0.f;
			round_flags = Side$(SideNW | SideNE | SideSE | SideSW);
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
				auto rr = round_radius * global_scale;
				auto st = shadow_thickness * global_scale;

				if (st > 0.f)
				{
					std::vector<Vec2f> points;
					path_rect(points, global_pos - Vec2f(st * 0.5f), global_size + Vec2f(st), rr, (Side$)round_flags);
					points.push_back(points[0]);
					canvas->stroke(points, Vec4c(0, 0, 0, 128), Vec4c(0), st);
				}
				if (alpha > 0.f)
				{
					std::vector<Vec2f> points;
					path_rect(points, global_pos, global_size, rr, (Side$)round_flags);
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
			copy->round_radius = round_radius;
			copy->round_flags = round_flags;
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
		float round_radius$;
		Side$ round_flags$m;
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
			round_radius$ = 0.f;
			round_flags$m = Side$(SideNW | SideNE | SideSE | SideSW);
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
			c->round_radius = round_radius$;
			c->round_flags = round_flags$m;
			c->frame_thickness = frame_thickness$;
			c->color = color$;
			c->frame_color = frame_color$;
			c->shadow_thickness = shadow_thickness$;
			c->clip_children = clip_children$;

			return c;
		}

		FLAME_UNIVERSE_EXPORTS void save$(Component* _c)
		{
			auto c = (cElement*)_c;

			pos$ = c->pos;
			scale$ = c->scale;
			size$ = c->size;
			inner_padding$ = c->inner_padding;
			alpha$ = c->alpha;
			round_radius$ = c->round_radius;
			round_flags$m = c->round_flags;
			frame_thickness$ = c->frame_thickness;
			color$ = c->color;
			frame_color$ = c->frame_color;
			shadow_thickness$ = c->shadow_thickness;
			clip_children$ = c->clip_children;
		}

		FLAME_UNIVERSE_EXPORTS void data_changed$(Component* _c, uint name_hash)
		{
			auto c = (cElement*)_c;

			switch (name_hash)
			{
			case cH("pos"):
				c->pos = pos$;
				break;
				break;
			case cH("scale"):
				c->scale = scale$;
				break;
			case cH("size"):
				c->size = size$;
				break;
			case cH("inner_padding"):
				c->inner_padding = inner_padding$;
				break;
			case cH("alpha"):
				c->alpha = alpha$;
				break;
			case cH("round_radius"):
				c->round_radius = round_radius$;
				break;
			case cH("round_flags"):
				c->round_flags = round_flags$m;
				break;
			case cH("frame_thickness"):
				c->frame_thickness = frame_thickness$;
				break;
			case cH("color"):
				c->color = color$;
				break;
			case cH("frame_color"):
				c->frame_color = frame_color$;
				break;
			case cH("shadow_thickness"):
				c->shadow_thickness = shadow_thickness$;
				break;
			case cH("clip_children"):
				c->clip_children = clip_children$;
				break;
			}
		}
	};
}
