#include "../entity_private.h"
#include "element_private.h"

#include "../renderpath/canvas_make_cmd/canvas.h"

namespace flame
{
	cElementPrivate::cElementPrivate()
	{
		pos_ = 0.f;
		scale = 1.f;
		size_ = 0.f;
		inner_padding_ = Vec4f(0.f);
		alpha = 1.f;
		roundness = Vec4f(0.f);
		frame_thickness = 0.f;
		color = Vec4c(0);
		frame_color = Vec4c(255);
		shadow_thickness = 0.f;
		clip_children = false;

		global_pos = 0.f;
		global_scale = 0.f;
		global_size = 0.f;
		cliped = true;
		cliped_rect = Vec4f(-1.f);
	}

	void cElementPrivate::calc_geometry()
	{
		auto p = entity->parent();
		if (!p)
		{
			global_pos = pos_;
			global_scale = scale;
		}
		else
		{
			auto p_element = p->get_component(Element);
			global_pos = p_element->global_pos + p_element->global_scale * pos_;
			global_scale = p_element->global_scale * scale;
		}
		global_size = size_ * global_scale;
	}

	void cElementPrivate::draw(graphics::Canvas* canvas)
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

	Component* cElementPrivate::copy()
	{
		auto copy = new cElementPrivate();

		copy->pos_ = pos_;
		copy->scale = scale;
		copy->size_ = size_;
		copy->inner_padding_ = inner_padding_;
		copy->alpha = alpha;
		copy->roundness = roundness;
		copy->frame_thickness = frame_thickness;
		copy->color = color;
		copy->frame_color = frame_color;
		copy->shadow_thickness = shadow_thickness;
		copy->clip_children = clip_children;

		return copy;
	}

	void cElement::set_x(float x, bool add, void* sender)
	{
		if (add && x == 0.f)
			return;
		if (x == pos_.x())
			return;
		if (add)
			pos_.x() += x;
		else
			pos_.x() = x;
		data_changed(cH("pos"), sender);
	}

	void cElement::set_y(float y, bool add, void* sender)
	{
		if (add && y == 0.f)
			return;
		if (y == pos_.y())
			return;
		if (add)
			pos_.y() += y;
		else
			pos_.y() = y;
		data_changed(cH("pos"), sender);
	}

	void cElement::set_pos(const Vec2f& p, bool add, void* sender)
	{
		if (add && p == 0.f)
			return;
		if (p == pos_)
			return;
		if (add)
			pos_ += p;
		else
			pos_ = p;
		data_changed(cH("pos"), sender);
	}

	void cElement::set_width(float w, bool add, void* sender)
	{
		if (add && w == 0.f)
			return;
		if (w == size_.x())
			return;
		if (add)
			size_.x() += w;
		else
			size_.x() = w;
		data_changed(cH("size"), sender);
	}

	void cElement::set_height(float h, bool add, void* sender)
	{
		if (add && h == 0.f)
			return;
		if (h == size_.y())
			return;
		if (add)
			size_.y() += h;
		else
			size_.y() = h;
		data_changed(cH("size"), sender);
	}

	void cElement::set_size(const Vec2f& s, bool add, void* sender)
	{
		if (add && s == 0.f)
			return;
		if (s == size_)
			return;
		if (add)
			size_ += s;
		else
			size_ = s;
		data_changed(cH("size"), sender);
	}

	cElement* cElement::create()
	{
		return new cElementPrivate();
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

		FLAME_UNIVERSE_EXPORTS Component* create$(Universe* u)
		{
			auto c = new cElementPrivate();

			c->pos_ = pos$;
			c->scale = scale$;
			c->size_ = size$;
			c->inner_padding_ = inner_padding$;
			c->alpha = alpha$;
			c->roundness = roundness$;
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
				pos$ = c->pos_;
				scale$ = c->scale;
				size$ = c->size_;
				inner_padding$ = c->inner_padding_;
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
					pos$ = c->pos_;
					break;
				case offsetof(ComponentElement$, scale$):
					scale$ = c->scale;
					break;
				case offsetof(ComponentElement$, size$):
					size$ = c->size_;
					break;
				case offsetof(ComponentElement$, inner_padding$):
					inner_padding$ = c->inner_padding_;
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
				c->pos_ = pos$;
				c->scale = scale$;
				c->size_ = size$;
				c->inner_padding_ = inner_padding$;
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
					c->pos_ = pos$;
					break;
				case offsetof(ComponentElement$, scale$):
					c->scale = scale$;
					break;
				case offsetof(ComponentElement$, size$):
					c->size_ = size$;
					break;
				case offsetof(ComponentElement$, inner_padding$):
					c->inner_padding_ = inner_padding$;
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
