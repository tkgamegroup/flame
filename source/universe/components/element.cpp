#include <flame/graphics/canvas.h>

#include "../world_private.h"
#include "element_private.h"
#include "../systems/element_renderer_private.h"

namespace flame
{
	void cElementPrivate::set_x(float _x)
	{
		if (x == _x)
			return;
		x = _x;
		mark_transform_dirty();
	}

	void cElementPrivate::set_y(float _y)
	{
		if (y == _y)
			return;
		y = _y;
		mark_transform_dirty();
	}

	void cElementPrivate::set_width(float w)
	{
		if (width == w)
			return;
		width = w;
		mark_transform_dirty();
	}

	void cElementPrivate::set_height(float h)
	{
		if (height == h)
			return;
		height = h;
		mark_transform_dirty();
	}

	void cElementPrivate::set_padding(const Vec4f& p)
	{
		if (padding == p)
			return;
		padding = p;
		mark_transform_dirty();
	}

	void cElementPrivate::set_pivotx(float p)
	{
		if (pivotx == p)
			return;
		pivotx = p;
		mark_transform_dirty();
	}

	void cElementPrivate::set_pivoty(float p)
	{
		if (pivoty == p)
			return;
		pivoty = p;
		mark_transform_dirty();
	}

	void cElementPrivate::set_scalex(float s)
	{
		if (scalex == s)
			return;
		scalex = s;
		mark_transform_dirty();
	}

	void cElementPrivate::set_scaley(float s)
	{
		if (scaley == s)
			return;
		scaley = s;
		mark_transform_dirty();
	}

	void cElementPrivate::set_rotation(float r)
	{
		if (rotation == r)
			return;
		rotation = r;
		mark_transform_dirty();
	}

	void cElementPrivate::set_skewx(float s)
	{
		if (skewx == s)
			return;
		skewx = s;
		mark_transform_dirty();
	}

	void cElementPrivate::set_skewy(float s)
	{
		if (skewy == s)
			return;
		skewy = s;
		mark_transform_dirty();
	}

	void cElementPrivate::update_transform()
	{
		if (!transform_dirty)
			return;
		transform_dirty = false;

		auto base_transform = Mat23f(1.f);
		auto p = ((EntityPrivate*)entity)->parent;
		if (p)
		{
			auto pe = (cElementPrivate*)p->get_component(cElement::type_hash);
			if (pe)
				base_transform = pe->get_transform();
		}

		auto axes = Mat2f(base_transform);
		auto c = Vec2f(base_transform[0][2], base_transform[1][2]) + 
			axes[0] * x + axes[1] * y;
		axes[0] = ::flame::rotation((rotation + skewy) * ANG_RAD) * axes[0] * scalex;
		axes[1] = ::flame::rotation((rotation + skewx) * ANG_RAD) * axes[1] * scaley;

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
		transform = Mat23f(Vec3f(axes[0], points[0].x()), Vec3f(axes[1], points[0].y()));
	}

	const Mat23f& cElementPrivate::get_transform()
	{
		update_transform();
		return transform;
	}

	Vec2f cElementPrivate::get_point(uint idx)
	{
		update_transform();
		return points[idx];
	}

	void cElementPrivate::set_fill_color(const Vec4c& c)
	{
		if (fill_color == c)
			return;
		fill_color = c;
		mark_drawing_dirty();
	}

	void cElementPrivate::set_border(float b)
	{
		if (border == b)
			return;
		border = b;
		mark_drawing_dirty();
	}

	void cElementPrivate::set_border_color(const Vec4c& c)
	{
		if (border_color == c)
			return;
		border_color = c;
		mark_drawing_dirty();
	}

	void cElementPrivate::set_clipping(bool c)
	{
		if (clipping == c)
			return;
		clipping = c;
		mark_drawing_dirty();
	}

	void cElementPrivate::mark_transform_dirty()
	{
		if (transform_dirty)
			return;
		transform_dirty = true;
		mark_drawing_dirty();
		for (auto& c : ((EntityPrivate*)entity)->children)
		{
			auto e = (cElementPrivate*)c->get_component(cElement::type_hash);
			e->mark_transform_dirty();
		}
	}

	void cElementPrivate::mark_drawing_dirty()
	{
		if (renderer)
			renderer->dirty = true;
	}

	bool cElementPrivate::contains(const Vec2f& p)
	{
		update_transform();
		Vec2f ps[] = { points[0], points[1], points[2], points[3] };
		return convex_contains<float>(p, ps);
	}

	void cElementPrivate::on_entered_world()
	{
		renderer = (sElementRendererPrivate*)((EntityPrivate*)entity)->world->get_system(sElementRenderer::type_hash);
		mark_transform_dirty();
	}

	void cElementPrivate::on_left_world()
	{
		mark_transform_dirty();
		renderer = nullptr;
	}

	void cElementPrivate::on_entity_visibility_changed()
	{
		mark_drawing_dirty();
	}

	void cElementPrivate::on_entity_position_changed()
	{
		mark_drawing_dirty();
	}

//	void cElementPrivate::calc_geometry()
//	{
//		if (global_scale != _global_scale)
//		{
//			mark_drawing_dirty();
//			global_scale = _global_scale;
//			data_changed(FLAME_CHASH("global_scale"), nullptr);
//		}
//		if (global_size != _global_size)
//		{
//			mark_drawing_dirty();
//			global_size = _global_size;
//			data_changed(FLAME_CHASH("global_size"), nullptr);
//		}
//		if (global_pos != _global_pos)
//		{
//			mark_drawing_dirty();
//			global_pos = _global_pos;
//			data_changed(FLAME_CHASH("global_pos"), nullptr);
//		}
//	}

	void cElementPrivate::draw_background(graphics::Canvas* canvas)
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

	void cElementPrivate::draw_content(graphics::Canvas* canvas)
	{
		for (auto d : drawers)
			d->draw(canvas);
	}

//	void cElement::set_pos(const Vec2f& p, void* sender)
//	{
//		data_changed(FLAME_CHASH("pos"), sender);
//	}
//
//	void cElement::set_scale(float s, void* sender)
//	{
//		data_changed(FLAME_CHASH("scale"), sender);
//	}
//
//	void cElement::set_size(const Vec2f& s, void* sender)
//	{
//		data_changed(FLAME_CHASH("size"), sender);
//	}
//
//	void cElement::set_alpha(float a, void* sender)
//	{
//		data_changed(FLAME_CHASH("alpha"), sender);
//	}
//
//	void cElement::set_roundness(const Vec4f& r, void* sender)
//	{
//		if (r == roundness)
//			return;
//		roundness = r;
//		mark_drawing_dirty();
//		data_changed(FLAME_CHASH("roundness"), sender);
//	}
//
//	void cElement::set_frame_thickness(float t, void* sender)
//	{
//		data_changed(FLAME_CHASH("frame_thickness"), sender);
//	}
//
//	void cElement::set_color(const Vec4c& c, void* sender)
//	{
//		data_changed(FLAME_CHASH("color"), sender);
//	}
//
//	void cElement::set_frame_color(const Vec4c& c, void* sender)
//	{
//		data_changed(FLAME_CHASH("frame_color"), sender);
//	}

	cElement* cElement::create()
	{
		return f_new<cElementPrivate>();
	}
}
