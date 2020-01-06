#include "../entity_private.h"
#include "element_private.h"

#include "../renderpath/canvas/canvas.h"

namespace flame
{
	cElementPrivate::cElementPrivate()
	{
		pos_ = 0.f;
		scale_ = 1.f;
		size_ = 0.f;
		inner_padding_ = Vec4f(0.f);
		alpha_ = 1.f;
		roundness_ = Vec4f(0.f);
		roundness_lod = 0;
		frame_thickness_ = 0.f;
		color_ = Vec4c(0);
		frame_color_ = Vec4c(255);
		clip_children = false;

		global_pos = 0.f;
		global_scale = 0.f;
		global_size = 0.f;
		cliped = true;
		cliped_rect = Vec4f(-1.f);

		cmds.impl = ListenerHubImpl::create();
	}

	cElementPrivate::~cElementPrivate()
	{
		ListenerHubImpl::destroy(cmds.impl);
	}

	void cElementPrivate::calc_geometry()
	{
		auto p = entity->parent();
		if (!p)
		{
			global_pos = pos_;
			global_scale = scale_;
		}
		else
		{
			auto p_element = p->get_component(cElement);
			global_pos = p_element->global_pos + p_element->global_scale * pos_;
			global_scale = p_element->global_scale * scale_;
		}
		global_size = size_ * global_scale;
	}

	void cElementPrivate::draw(graphics::Canvas* canvas)
	{
		if (!cliped)
		{
			auto r = roundness_ * global_scale;

			if (alpha_ > 0.f)
			{
				std::vector<Vec2f> points;
				path_rect(points, global_pos, global_size, r, roundness_lod);
				if (color_.w() > 0)
					canvas->fill(points, alpha_mul(color_, alpha_));
				auto ft = frame_thickness_ * global_scale;
				if (ft > 0.f && frame_color_.w() > 0)
				{
					points.push_back(points[0]);
					canvas->stroke(points, alpha_mul(frame_color_, alpha_), ft);
				}
			}
		}

		cmds.call(canvas);
	}

	Component* cElementPrivate::copy()
	{
		auto copy = new cElementPrivate();

		copy->pos_ = pos_;
		copy->scale_ = scale_;
		copy->size_ = size_;
		copy->inner_padding_ = inner_padding_;
		copy->alpha_ = alpha_;
		copy->roundness_ = roundness_;
		copy->roundness_lod = roundness_lod;
		copy->frame_thickness_ = frame_thickness_;
		copy->color_ = color_;
		copy->frame_color_ = frame_color_;
		copy->clip_children = clip_children;

		return copy;
	}

	void cElement::set_x(float x, bool add, void* sender)
	{
		if (add && x == 0.f)
			return;
		if (!add && x == pos_.x())
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
		if (!add && y == pos_.y())
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
		if (!add && p == pos_)
			return;
		if (add)
			pos_ += p;
		else
			pos_ = p;
		data_changed(cH("pos"), sender);
	}

	void cElement::set_scale(float s, bool add, void* sender)
	{
		if (add && s == 0.f)
			return;
		if (!add && s == scale_)
			return;
		if (add)
			scale_ += s;
		else
			scale_ = s;
		data_changed(cH("scale"), sender);
	}

	void cElement::set_width(float w, bool add, void* sender)
	{
		if (add && w == 0.f)
			return;
		if (!add && w == size_.x())
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
		if (!add && h == size_.y())
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
		if (!add && s == size_)
			return;
		if (add)
			size_ += s;
		else
			size_ = s;
		data_changed(cH("size"), sender);
	}

	void cElement::set_alpha(float a, bool add, void* sender)
	{
		if (add && a == 0.f)
			return;
		if (!add && a == alpha_)
			return;
		if (add)
			alpha_ += a;
		else
			alpha_ = a;
		data_changed(cH("alpha"), sender);
	}

	void cElement::set_roundness(const Vec4f& r, bool add, void* sender)
	{
		if (add && r == 0.f)
			return;
		if (!add && r == roundness_)
			return;
		if (add)
			roundness_ += r;
		else
			roundness_ = r;
		data_changed(cH("roundness"), sender);
	}

	void cElement::set_frame_thickness(float t, bool add, void* sender)
	{
		if (add && t == 0.f)
			return;
		if (!add && t == frame_thickness_)
			return;
		if (add)
			frame_thickness_ += t;
		else
			frame_thickness_ = t;
		data_changed(cH("frame_thickness"), sender);
	}

	void cElement::set_color(const Vec4c& c, bool add, void* sender)
	{
		if (add && c == (uchar)0)
			return;
		if (!add && c == color_)
			return;
		if (add)
			color_ += c;
		else
			color_ = c;
		data_changed(cH("color"), sender);
	}

	void cElement::set_frame_color(const Vec4c& c, bool add, void* sender)
	{
		if (add && c == (uchar)0)
			return;
		if (!add && c == frame_color_)
			return;
		if (add)
			frame_color_ += c;
		else
			frame_color_ = c;
		data_changed(cH("frame_color"), sender);
	}

	cElement* cElement::create()
	{
		return new cElementPrivate();
	}

	struct Serializer_cElement$
	{
		Vec2f pos$;
		float scale$;
		Vec2f size$;
		Vec4f inner_padding$;
		float alpha$;
		Vec4f roundness$;
		uint roundness_lod$;
		float frame_thickness$;
		Vec4c color$;
		Vec4c frame_color$;
		bool clip_children$;

		FLAME_UNIVERSE_EXPORTS Serializer_cElement$()
		{
			pos$ = 0.f;
			scale$ = 1.f;
			size$ = 0.f;
			inner_padding$ = Vec4f(0.f);
			alpha$ = 1.f;
			roundness$ = Vec4f(0.f);
			roundness_lod$ = 0;
			frame_thickness$ = 0.f;
			color$ = Vec4c(0);
			frame_color$ = Vec4c(255);
			clip_children$ = false;
		}

		FLAME_UNIVERSE_EXPORTS Component* create$(World* w)
		{
			auto c = new cElementPrivate();

			c->pos_ = pos$;
			c->scale_ = scale$;
			c->size_ = size$;
			c->inner_padding_ = inner_padding$;
			c->alpha_ = alpha$;
			c->roundness_ = roundness$;
			c->roundness_lod = roundness_lod$;
			c->frame_thickness_ = frame_thickness$;
			c->color_ = color$;
			c->frame_color_ = frame_color$;
			c->clip_children = clip_children$;

			return c;
		}

		FLAME_UNIVERSE_EXPORTS void serialize$(Component* _c, int offset)
		{
			auto c = (cElementPrivate*)_c;

			if (offset == -1)
			{
				pos$ = c->pos_;
				scale$ = c->scale_;
				size$ = c->size_;
				inner_padding$ = c->inner_padding_;
				alpha$ = c->alpha_;
				roundness$ = c->roundness_;
				roundness_lod$ = c->roundness_lod;
				frame_thickness$ = c->frame_thickness_;
				color$ = c->color_;
				frame_color$ = c->frame_color_;
				clip_children$ = c->clip_children;
			}
			else
			{
				switch (offset)
				{
				case offsetof(Serializer_cElement$, pos$):
					pos$ = c->pos_;
					break;
				case offsetof(Serializer_cElement$, scale$):
					scale$ = c->scale_;
					break;
				case offsetof(Serializer_cElement$, size$):
					size$ = c->size_;
					break;
				case offsetof(Serializer_cElement$, inner_padding$):
					inner_padding$ = c->inner_padding_;
					break;
				case offsetof(Serializer_cElement$, alpha$):
					alpha$ = c->alpha_;
					break;
				case offsetof(Serializer_cElement$, roundness$):
					roundness$ = c->roundness_;
					break;
				case offsetof(Serializer_cElement$, roundness_lod$):
					roundness_lod$ = c->roundness_lod;
					break;
				case offsetof(Serializer_cElement$, frame_thickness$):
					frame_thickness$ = c->frame_thickness_;
					break;
				case offsetof(Serializer_cElement$, color$):
					color$ = c->color_;
					break;
				case offsetof(Serializer_cElement$, frame_color$):
					frame_color$ = c->frame_color_;
					break;
				case offsetof(Serializer_cElement$, clip_children$):
					clip_children$ = c->clip_children;
					break;
				}
			}
		}

		FLAME_UNIVERSE_EXPORTS void unserialize$(Component* _c, int offset)
		{
			auto c = (cElementPrivate*)_c;

			if (offset == -1)
			{
				c->pos_ = pos$;
				c->scale_ = scale$;
				c->size_ = size$;
				c->inner_padding_ = inner_padding$;
				c->alpha_ = alpha$;
				c->roundness_ = roundness$;
				c->roundness_lod = roundness_lod$;
				c->frame_thickness_ = frame_thickness$;
				c->color_ = color$;
				c->frame_color_ = frame_color$;
				c->clip_children = clip_children$;
			}
			else
			{
				switch (offset)
				{
				case offsetof(Serializer_cElement$, pos$):
					c->pos_ = pos$;
					break;
				case offsetof(Serializer_cElement$, scale$):
					c->scale_ = scale$;
					break;
				case offsetof(Serializer_cElement$, size$):
					c->size_ = size$;
					break;
				case offsetof(Serializer_cElement$, inner_padding$):
					c->inner_padding_ = inner_padding$;
					break;
				case offsetof(Serializer_cElement$, alpha$):
					c->alpha_ = alpha$;
					break;
				case offsetof(Serializer_cElement$, roundness$):
					c->roundness_ = roundness$;
					break;
				case offsetof(Serializer_cElement$, roundness_lod$):
					c->roundness_lod = roundness_lod$;
					break;
				case offsetof(Serializer_cElement$, frame_thickness$):
					c->frame_thickness_ = frame_thickness$;
					break;
				case offsetof(Serializer_cElement$, color$):
					c->color_ = color$;
					break;
				case offsetof(Serializer_cElement$, frame_color$):
					c->frame_color_ = frame_color$;
					break;
				case offsetof(Serializer_cElement$, clip_children$):
					c->clip_children = clip_children$;
					break;
				}
			}
		}
	};
}
