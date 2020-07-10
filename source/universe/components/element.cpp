#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "../world_private.h"
#include "../systems/element_renderer_private.h"
#include "element_private.h"

namespace flame
{
	void cElementPrivate::set_x(float _x)
	{
		x = _x;
		mark_transform_dirty();
	}

	void cElementPrivate::set_y(float _y)
	{
		y = _y;
		mark_transform_dirty();
	}

	void cElementPrivate::set_width(float w)
	{
		width = w;
		mark_transform_dirty();
	}

	void cElementPrivate::set_height(float h)
	{
		height = h;
		mark_transform_dirty();
	}

	void cElementPrivate::set_pivotx(float p)
	{
		pivotx = p;
		mark_transform_dirty();
	}

	void cElementPrivate::set_pivoty(float p)
	{
		pivoty = p;
		mark_transform_dirty();
	}

	void cElementPrivate::set_scalex(float s)
	{
		scalex = s;
		mark_transform_dirty();
	}

	void cElementPrivate::set_scaley(float s)
	{
		scaley = s;
		mark_transform_dirty();
	}

	void cElementPrivate::set_rotation(float r)
	{
		rotation = r;
		mark_transform_dirty();
	}

	void cElementPrivate::set_skewx(float s)
	{
		skewx = s;
		mark_transform_dirty();
	}

	void cElementPrivate::set_skewy(float s)
	{
		skewy = s;
		mark_transform_dirty();
	}

	void cElementPrivate::update_transform()
	{
		if (!transform_dirty)
			return;
		transform_dirty = false;

		auto base_transform = Mat<3, 2, float>(1.f);
		auto p = ((EntityPrivate*)entity)->parent;
		if (p)
		{
			auto pe = (cElementPrivate*)p->get_component(cElement::type_hash);
			if (pe)
				base_transform = pe->get_transform();
		}

		auto axis = Mat2f(base_transform);
		auto c = Vec2f(base_transform[0][2], base_transform[1][2]) + 
			axis[0] * x + axis[1] * y;
		axis[0] = ::flame::rotation((rotation + skewy) * ANG_RAD) * axis[0] * scalex;
		axis[1] = ::flame::rotation((rotation + skewx) * ANG_RAD) * axis[1] * scaley;

		auto w = axis[0] * width;
		auto h = axis[1] * height;
		p00 = w * -pivotx + h * -pivoty + c;
		p10 = w * (1.f - pivotx) + h * -pivoty + c;
		p01 = w * -pivotx + h * (1.f - pivoty) + c;
		p11 = w * (1.f - pivotx) + h * (1.f - pivoty) + c;
		transform = Mat<3, 2, float>(Vec3f(axis[0], p00.x()), Vec3f(axis[1], p00.y()));
	}

	const Mat<3, 2, float>& cElementPrivate::get_transform()
	{
		if (transform_dirty)
			update_transform();
		return transform;
	}

	Vec2f cElementPrivate::get_p00()
	{
		if (transform_dirty)
			update_transform();
		return p00;
	}

	Vec2f cElementPrivate::get_p10()
	{
		if (transform_dirty)
			update_transform();
		return p10;
	}

	Vec2f cElementPrivate::get_p11()
	{
		if (transform_dirty)
			update_transform();
		return p11;
	}

	Vec2f cElementPrivate::get_p01()
	{
		if (transform_dirty)
			update_transform();
		return p01;
	}

	void cElementPrivate::set_fill_color(const Vec3c& c)
	{
		fill_color = c;
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

	void cElementPrivate::on_entity_position_changed()
	{
		mark_drawing_dirty();
	}

//	void cElementPrivate::calc_geometry()
//	{
//		float _global_scale;
//		Vec2f _global_size;
//		Vec2f _global_pos;
//
//		auto p = entity->parent;
//		if (!p)
//		{
//			_global_scale = scale;
//			_global_size = size * global_scale;
//			_global_pos = pos;
//		}
//		else
//		{
//			auto p_element = p->get_component(cElement);
//			_global_scale = p_element->global_scale * scale;
//			_global_size = size * global_scale;
//			_global_pos = p_element->global_pos + p_element->global_scale * pos - pivot * global_size;
//		}
//
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
//
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
//		if (!clipped)
//		{
//			if (alpha > 0.f)
//			{
				std::vector<Vec2f> points;
				points.push_back(get_p00());
				points.push_back(get_p10());
				points.push_back(get_p11());
				points.push_back(get_p01());
				canvas->fill(points.size(), points.data(), Vec4c(fill_color, 255));

				for (auto d : drawers)
					d->draw(canvas);
//				auto p = floor(global_pos);
//				auto s = floor(global_size);
//				auto r = floor(roundness * global_scale);
//				path_rect(points, p, s, r, roundness_lod);
//				if (color.w() > 0)
//					canvas->fill(points.size(), points.data(), color.copy().factor_w(alpha));
//				auto ft = frame_thickness * global_scale;
//				if (ft > 0.f && frame_color.w() > 0)
//				{
//					points.clear();
//					path_rect(points, p + 0.5f, s - 0.5f, r, roundness_lod);
//					points.push_back(points[0]);
//					canvas->stroke(points.size(), points.data(), frame_color.copy().factor_w(alpha), ft);
//				}
//			}
//		}
	}
//
//	void cElementPrivate::on_event(EntityEvent e, void* t)
//	{
//		switch (e)
//		{
//		case EntityEnteredWorld:
//		{
//			calc_geometry();
//			renderer = entity->world->get_system(sElementRenderer);
//			renderer->pending_update = true;
//		}
//			break;
//		case EntityLeftWorld:
//			mark_drawing_dirty();
//			renderer = nullptr;
//			break;
//		case EntityVisibilityChanged:
//			calc_geometry();
//			mark_drawing_dirty();
//			break;
//		case EntityPositionChanged:
//			mark_drawing_dirty();
//			break;
//		}
//	}
//
//	void cElement::set_pos(const Vec2f& p, void* sender)
//	{
//		if (p == pos)
//			return;
//		pos = p;
//		mark_drawing_dirty();
//		data_changed(FLAME_CHASH("pos"), sender);
//	}
//
//	void cElement::set_scale(float s, void* sender)
//	{
//		if (s == scale)
//			return;
//		scale = s;
//		mark_drawing_dirty();
//		data_changed(FLAME_CHASH("scale"), sender);
//	}
//
//	void cElement::set_size(const Vec2f& s, void* sender)
//	{
//		if (s == size)
//			return;
//		size = s;
//		mark_drawing_dirty();
//		data_changed(FLAME_CHASH("size"), sender);
//	}
//
//	void cElement::set_alpha(float a, void* sender)
//	{
//		if (a == alpha)
//			return;
//		alpha = a;
//		mark_drawing_dirty();
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
//		if (t == frame_thickness)
//			return;
//		frame_thickness = t;
//		mark_drawing_dirty();
//		data_changed(FLAME_CHASH("frame_thickness"), sender);
//	}
//
//	void cElement::set_color(const Vec4c& c, void* sender)
//	{
//		if (c == color)
//			return;
//		color = c;
//		mark_drawing_dirty();
//		data_changed(FLAME_CHASH("color"), sender);
//	}
//
//	void cElement::set_frame_color(const Vec4c& c, void* sender)
//	{
//		if (c == frame_color)
//			return;
//		frame_color = c;
//		mark_drawing_dirty();
//		data_changed(FLAME_CHASH("frame_color"), sender);
//	}

	cElementPrivate* cElementPrivate::create()
	{
		auto ret = _allocate(sizeof(cElementPrivate));
		new (ret) cElementPrivate;
		return (cElementPrivate*)ret;
	}

	cElement* cElement::create() { return cElementPrivate::create(); }
}
