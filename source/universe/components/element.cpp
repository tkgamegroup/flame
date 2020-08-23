#include <flame/graphics/canvas.h>
#include "../world_private.h"
#include "element_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cElementPrivate::set_x(float _x)
	{
		if (x == _x)
			return;
		x = _x;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<ch("x")>::v);
	}

	void cElementPrivate::set_y(float _y)
	{
		if (y == _y)
			return;
		y = _y;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<ch("y")>::v);
	}

	void cElementPrivate::set_width(float w)
	{
		if (width == w)
			return;
		width = w;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<ch("width")>::v);
	}

	void cElementPrivate::set_height(float h)
	{
		if (height == h)
			return;
		height = h;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<ch("height")>::v);
	}

	void cElementPrivate::set_padding(const Vec4f& p)
	{
		if (padding == p)
			return;
		padding = p;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<ch("padding")>::v);
	}

	void cElementPrivate::set_pivotx(float p)
	{
		if (pivotx == p)
			return;
		pivotx = p;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<ch("pivotx")>::v);
	}

	void cElementPrivate::set_pivoty(float p)
	{
		if (pivoty == p)
			return;
		pivoty = p;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<ch("pivoty")>::v);
	}

	void cElementPrivate::set_scalex(float s)
	{
		if (scalex == s)
			return;
		scalex = s;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<ch("scalex")>::v);
	}

	void cElementPrivate::set_scaley(float s)
	{
		if (scaley == s)
			return;
		scaley = s;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<ch("scaley")>::v);
	}

	void cElementPrivate::set_rotation(float r)
	{
		if (rotation == r)
			return;
		rotation = r;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<ch("rotation")>::v);
	}

	void cElementPrivate::set_skewx(float s)
	{
		if (skewx == s)
			return;
		skewx = s;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<ch("skewx")>::v);
	}

	void cElementPrivate::set_skewy(float s)
	{
		if (skewy == s)
			return;
		skewy = s;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<ch("skewy")>::v);
	}

	void cElementPrivate::update_transform()
	{
		if (!transform_dirty)
			return;
		transform_dirty = false;

		auto base_axes = Mat2f(1.f);
		auto base_pos = Vec2f(0.f);
		auto p = ((EntityPrivate*)entity)->parent;
		if (p)
		{
			auto pe = p->get_component_t<cElementPrivate>();
			if (pe)
			{
				pe->update_transform();
				base_axes = pe->axes;
				base_pos = pe->points[0];
			}
		}

		axes = base_axes;
		auto c = base_pos + axes[0] * x + axes[1] * y;
		axes[0] = get_rotation_matrix((rotation + skewy) * ANG_RAD) * axes[0] * scalex;
		axes[1] = get_rotation_matrix((rotation + skewx) * ANG_RAD) * axes[1] * scaley;

		points[0] = c + axes * Vec2f(-pivotx * width, -pivoty * height);
		points[1] = c + axes * Vec2f((1.f - pivotx) * width, -pivoty * height);
		points[2] = c + axes * Vec2f((1.f - pivotx) * width, (1.f - pivoty) * height);
		points[3] = c + axes * Vec2f(-pivotx * width, (1.f - pivoty) * height);

		auto pl = min(padding[0], width * 0.5f);
		auto pt = min(padding[1], height * 0.5f);
		auto pr = -min(padding[2], width * 0.5f);
		auto pb = -min(padding[3], height * 0.5f);
		points[4] = points[0] + axes * Vec2f(pl, pt);
		points[5] = points[1] + axes * Vec2f(pr, pt);
		points[6] = points[2] + axes * Vec2f(pr, pb);
		points[7] = points[3] + axes * Vec2f(pl, pb);

		points[8] = (points[0] + points[1] + points[2] + points[3]) * 0.25f;
		points[9] = (points[4] + points[5] + points[6] + points[7]) * 0.25f;

		aabb.x() = points[0].x();
		aabb.z() = points[0].x();
		aabb.y() = points[0].y();
		aabb.w() = points[0].y();
		for (auto i = 1; i < 4; i++)
		{
			aabb.x() = min(aabb.x(), points[i].x());
			aabb.z() = max(aabb.z(), points[i].x());
			aabb.y() = min(aabb.y(), points[i].y());
			aabb.w() = max(aabb.w(), points[i].y());
		}
	}

	Vec2f cElementPrivate::get_point(uint idx)
	{
		update_transform();
		return points[idx];
	}

	Mat2f cElementPrivate::get_axes()
	{
		update_transform();
		return axes;
	}

	void cElementPrivate::set_fill_color(const Vec4c& c)
	{
		if (fill_color == c)
			return;
		fill_color = c;
		mark_drawing_dirty();
		Entity::report_data_changed(this, S<ch("fill_color")>::v);
	}

	void cElementPrivate::set_border(float b)
	{
		if (border == b)
			return;
		border = b;
		mark_drawing_dirty();
		Entity::report_data_changed(this, S<ch("border")>::v);
	}

	void cElementPrivate::set_border_color(const Vec4c& c)
	{
		if (border_color == c)
			return;
		border_color = c;
		mark_drawing_dirty();
		Entity::report_data_changed(this, S<ch("border_color")>::v);
	}

	void cElementPrivate::set_clipping(bool c)
	{
		if (clipping == c)
			return;
		clipping = c;
		mark_drawing_dirty();
		Entity::report_data_changed(this, S<ch("clipping")>::v);
	}

	void cElementPrivate::mark_transform_dirty()
	{
		if (!transform_dirty)
		{
			transform_dirty = true;
			for (auto& c : ((EntityPrivate*)entity)->children)
			{
				auto e = c->get_component_t<cElementPrivate>();
				e->mark_transform_dirty();
			}
		}
		mark_drawing_dirty();
	}

	void cElementPrivate::mark_drawing_dirty()
	{
		if (renderer)
			renderer->dirty = true;
	}

	void cElementPrivate::on_gain_renderer()
	{
		mark_transform_dirty();
	}

	void cElementPrivate::on_lost_renderer()
	{
		mark_transform_dirty();
	}

	bool cElementPrivate::contains(const Vec2f& p)
	{
		if (width == 0.f || height == 0.f)
			return false;
		update_transform();
		Vec2f ps[] = { points[0], points[1], points[2], points[3] };
		return convex_contains<float>(p, ps);
	}

	void cElementPrivate::on_local_message(Message msg, void* p)
	{
		switch (msg)
		{
		case MessageVisibilityChanged:
		case MessagePositionChanged:
		case MessageElementTransformDirty:
			mark_transform_dirty();
			break;
		case MessageElementDrawingDirty:
			mark_drawing_dirty();
			break;
		}
	}

	void cElementPrivate::draw(graphics::Canvas* canvas)
	{
//#ifdef _DEBUG
//		if (debug_level > 0)
//		{
//			debug_break();
//			debug_level = 0;
//		}
//#endif
//
		//if (alpha > 0.f)
		{
			if (fill_color.a() > 0)
			{
				canvas->begin_path();
				canvas->move_to(points[0]);
				canvas->line_to(points[1]);
				canvas->line_to(points[2]);
				canvas->line_to(points[3]);
				canvas->fill(fill_color);
			}

			if (border > 0.f && border_color.a() > 0)
			{
				auto hf = border * 0.5f;
				canvas->begin_path();
				canvas->move_to(points[0] + Vec2f(hf, hf));
				canvas->line_to(points[1] + Vec2f(-hf, hf));
				canvas->line_to(points[2] + Vec2f(-hf, -hf));
				canvas->line_to(points[3] + Vec2f(hf, -hf));
				canvas->line_to(points[0] + Vec2f(hf, hf));
				canvas->stroke(border_color, border);
			}
		}
	}

//	void cElement::set_roundness(const Vec4f& r, void* sender)
//	{
//		if (r == roundness)
//			return;
//		roundness = r;
//		mark_drawing_dirty();
//		report_data_changed(FLAME_CHASH("roundness"), sender);
//	}

	cElement* cElement::create()
	{
		return f_new<cElementPrivate>();
	}
}
