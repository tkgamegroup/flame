#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "../world_private.h"
#include "../systems/element_renderer_private.h"
#include "element_private.h"

namespace flame
{
	void cElementPrivate::_set_x(float x)
	{
		_x = x;
		_transform_dirty = true;
		if (_renderer)
			_renderer->_dirty = true;
	}

	void cElementPrivate::_set_y(float y)
	{
		_y = y;
		_transform_dirty = true;
		if (_renderer)
			_renderer->_dirty = true;
	}

	void cElementPrivate::_set_width(float w)
	{
		_width = w;
		_transform_dirty = true;
		if (_renderer)
			_renderer->_dirty = true;
	}

	void cElementPrivate::_set_height(float h)
	{
		_height = h;
		_transform_dirty = true;
		if (_renderer)
			_renderer->_dirty = true;
	}

	void cElementPrivate::_set_pivotx(float p)
	{
		_pivotx = p;
		_transform_dirty = true;
		if (_renderer)
			_renderer->_dirty = true;
	}

	void cElementPrivate::_set_pivoty(float p)
	{
		_pivoty = p;
		_transform_dirty = true;
		if (_renderer)
			_renderer->_dirty = true;
	}

	void cElementPrivate::_set_scalex(float s)
	{
		_scalex = s;
		_transform_dirty = true;
		if (_renderer)
			_renderer->_dirty = true;
	}

	void cElementPrivate::_set_scaley(float s)
	{
		_scaley = s;
		_transform_dirty = true;
		if (_renderer)
			_renderer->_dirty = true;
	}

	void cElementPrivate::_set_rotation(float r)
	{
		_rotation = r;
		_transform_dirty = true;
		if (_renderer)
			_renderer->_dirty = true;
	}

	void cElementPrivate::_set_skewx(float s)
	{
		_skewx = s;
		_transform_dirty = true;
		if (_renderer)
			_renderer->_dirty = true;
	}

	void cElementPrivate::_set_skewy(float s)
	{
		_skewy = s;
		_transform_dirty = true;
		if (_renderer)
			_renderer->_dirty = true;
	}


	void cElementPrivate::_update_transform()
	{
		if (!_transform_dirty)
			return;
		_transform_dirty = false;

		auto base_transform = Mat<3, 2, float>(1.f);
		auto p = ((EntityPrivate*)entity)->_parent;
		if (p)
		{
			auto pe = (cElementPrivate*)p->_get_component(std::hash<std::string>()("cElement"));
			if (pe)
				base_transform = pe->_get_transform();
		}

		auto axis = Mat2f(base_transform);
		auto c = Vec2f(base_transform[0][2], base_transform[1][2]) + 
			axis[0] * _x + axis[1] * _y;
		axis[0] = rotation((_rotation + _skewy) * ANG_RAD) * axis[0] * _scalex;
		axis[1] = rotation((_rotation + _skewx) * ANG_RAD) * axis[1] * _scaley;

		auto w = axis[0] * _width;
		auto h = axis[1] * _height;
		_p00 = w * -_pivotx + h * -_pivoty + c;
		_p10 = w * (1.f - _pivotx) + h * -_pivoty + c;
		_p01 = w * -_pivotx + h * (1.f - _pivoty) + c;
		_p11 = w * (1.f - _pivotx) + h * (1.f - _pivoty) + c;
		_transform = Mat<3, 2, float>(Vec3f(axis[0], _p00.x()), Vec3f(axis[1], _p00.y()));
	}

	const Mat<3, 2, float>& cElementPrivate::_get_transform()
	{
		if (_transform_dirty)
			_update_transform();
		return _transform;
	}

	const Vec2f& cElementPrivate::_get_p00()
	{
		if (_transform_dirty)
			_update_transform();
		return _p00;
	}

	const Vec2f& cElementPrivate::_get_p10()
	{
		if (_transform_dirty)
			_update_transform();
		return _p10;
	}

	const Vec2f& cElementPrivate::_get_p11()
	{
		if (_transform_dirty)
			_update_transform();
		return _p11;
	}

	const Vec2f& cElementPrivate::_get_p01()
	{
		if (_transform_dirty)
			_update_transform();
		return _p01;
	}

	void cElementPrivate::_set_fill_color(const Vec3c& c)
	{
		_fill_color = c;
		if (_renderer)
			_renderer->_dirty = true;
	}

	void cElementPrivate::_on_entered_world()
	{
		_renderer = (sElementRendererPrivate*)((EntityPrivate*)entity)->_world->_get_system(FLAME_CHASH("sElementRenderer"));
		if (_renderer)
			_renderer->_dirty = true;
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
//			mark_dirty();
//			global_scale = _global_scale;
//			data_changed(FLAME_CHASH("global_scale"), nullptr);
//		}
//		if (global_size != _global_size)
//		{
//			mark_dirty();
//			global_size = _global_size;
//			data_changed(FLAME_CHASH("global_size"), nullptr);
//		}
//		if (global_pos != _global_pos)
//		{
//			mark_dirty();
//			global_pos = _global_pos;
//			data_changed(FLAME_CHASH("global_pos"), nullptr);
//		}
//	}
//
	void cElementPrivate::_draw(graphics::Canvas* canvas)
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
				points.push_back(_get_p00());
				points.push_back(_get_p10());
				points.push_back(_get_p11());
				points.push_back(_get_p01());
				canvas->fill(points.size(), points.data(), Vec4c(_fill_color, 255));
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
//			mark_dirty();
//			renderer = nullptr;
//			break;
//		case EntityVisibilityChanged:
//			calc_geometry();
//			mark_dirty();
//			break;
//		case EntityPositionChanged:
//			mark_dirty();
//			break;
//		}
//	}
//
//	void cElement::set_pos(const Vec2f& p, void* sender)
//	{
//		if (p == pos)
//			return;
//		pos = p;
//		mark_dirty();
//		data_changed(FLAME_CHASH("pos"), sender);
//	}
//
//	void cElement::set_scale(float s, void* sender)
//	{
//		if (s == scale)
//			return;
//		scale = s;
//		mark_dirty();
//		data_changed(FLAME_CHASH("scale"), sender);
//	}
//
//	void cElement::set_size(const Vec2f& s, void* sender)
//	{
//		if (s == size)
//			return;
//		size = s;
//		mark_dirty();
//		data_changed(FLAME_CHASH("size"), sender);
//	}
//
//	void cElement::set_alpha(float a, void* sender)
//	{
//		if (a == alpha)
//			return;
//		alpha = a;
//		mark_dirty();
//		data_changed(FLAME_CHASH("alpha"), sender);
//	}
//
//	void cElement::set_roundness(const Vec4f& r, void* sender)
//	{
//		if (r == roundness)
//			return;
//		roundness = r;
//		mark_dirty();
//		data_changed(FLAME_CHASH("roundness"), sender);
//	}
//
//	void cElement::set_frame_thickness(float t, void* sender)
//	{
//		if (t == frame_thickness)
//			return;
//		frame_thickness = t;
//		mark_dirty();
//		data_changed(FLAME_CHASH("frame_thickness"), sender);
//	}
//
//	void cElement::set_color(const Vec4c& c, void* sender)
//	{
//		if (c == color)
//			return;
//		color = c;
//		mark_dirty();
//		data_changed(FLAME_CHASH("color"), sender);
//	}
//
//	void cElement::set_frame_color(const Vec4c& c, void* sender)
//	{
//		if (c == frame_color)
//			return;
//		frame_color = c;
//		mark_dirty();
//		data_changed(FLAME_CHASH("frame_color"), sender);
//	}

	cElementPrivate* cElementPrivate::_create()
	{
		auto ret = _allocate(sizeof(cElementPrivate));
		new (ret) cElementPrivate;
		return (cElementPrivate*)ret;
	}

	cElement* cElement::create() { return cElementPrivate::_create(); }
}
