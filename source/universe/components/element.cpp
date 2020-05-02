#include <flame/graphics/canvas.h>
#include  "../world_private.h"
#include "element_private.h"

namespace flame
{
	cElementPrivate::cElementPrivate()
	{
		renderer = nullptr;

		pos = 0.f;
		size = 0.f;
		scale = 1.f;
		pivot = 0.f;
		padding = Vec4f(0.f);
		alpha = 1.f;
		roundness = Vec4f(0.f);
		roundness_lod = 0;
		frame_thickness = 0.f;
		color = Vec4c(0);
		frame_color = Vec4c(255);
		clip_flags = (ClipFlag)0;

		global_pos = 0.f;
		global_scale = 0.f;
		global_size = 0.f;
		clipped = false;
		clipped_rect = Vec4f(-1.f);

		cmds.impl = ListenerHubImpl::create();

		last_global_pos = 0.f;
		last_global_scale = 0.f;
		last_global_size = 0.f;
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
			global_scale = scale;
			global_size = size * global_scale;
			global_pos = pos;
		}
		else
		{
			auto p_element = p->get_component(cElement);
			global_scale = p_element->global_scale * scale;
			global_size = size * global_scale;
			global_pos = p_element->global_pos + p_element->global_scale * pos - pivot * global_size;
		}

		if (global_pos != last_global_pos)
		{
			mark_dirty();
			last_global_pos = global_pos;
			data_changed(FLAME_CHASH("global_pos"), nullptr);
		}
		if (global_scale != last_global_scale)
		{
			mark_dirty();
			last_global_scale = global_scale;
			data_changed(FLAME_CHASH("global_scale"), nullptr);
		}
		if (global_size != last_global_size)
		{
			mark_dirty();
			last_global_size = global_size;
			data_changed(FLAME_CHASH("global_size"), nullptr);
		}
	}

	void cElementPrivate::draw(graphics::Canvas* canvas)
	{
#ifdef _DEBUG
		if (debug_level > 0)
		{
			debug_break();
			debug_level = 0;
		}
#endif

		if (!clipped)
		{
			if (alpha > 0.f)
			{
				std::vector<Vec2f> points;
				auto p = floor(global_pos);
				auto s = floor(global_size);
				auto r = floor(roundness * global_scale);
				path_rect(points, p, s, r, roundness_lod);
				if (color.w() > 0)
					canvas->fill(points.size(), points.data(), color.copy().factor_w(alpha));
				auto ft = frame_thickness * global_scale;
				if (ft > 0.f && frame_color.w() > 0)
				{
					points.clear();
					path_rect(points, p + 0.5f, s - 0.5f, r, roundness_lod);
					points.push_back(points[0]);
					canvas->stroke(points.size(), points.data(), frame_color.copy().factor_w(alpha), ft);
				}
			}
		}
	}

	void cElementPrivate::on_entered_world()
	{
		calc_geometry();
		renderer = entity->world()->get_system(s2DRenderer);
		renderer->pending_update = true;
	}

	void cElementPrivate::on_left_world()
	{
		renderer->pending_update = true;
		renderer = nullptr;
	}

	void cElementPrivate::on_visibility_changed()
	{
		calc_geometry();
		mark_dirty();
	}

	void cElementPrivate::on_position_changed()
	{
		mark_dirty();
	}

	void cElement::set_pos(const Vec2f& p, void* sender)
	{
		if (p == pos)
			return;
		pos = p;
		mark_dirty();
		data_changed(FLAME_CHASH("pos"), sender);
	}

	void cElement::set_scale(float s, void* sender)
	{
		if (s == scale)
			return;
		scale = s;
		mark_dirty();
		data_changed(FLAME_CHASH("scale"), sender);
	}

	void cElement::set_size(const Vec2f& s, void* sender)
	{
		if (s == size)
			return;
		size = s;
		mark_dirty();
		data_changed(FLAME_CHASH("size"), sender);
	}

	void cElement::set_alpha(float a, void* sender)
	{
		if (a == alpha)
			return;
		alpha = a;
		mark_dirty();
		data_changed(FLAME_CHASH("alpha"), sender);
	}

	void cElement::set_roundness(const Vec4f& r, void* sender)
	{
		if (r == roundness)
			return;
		roundness = r;
		mark_dirty();
		data_changed(FLAME_CHASH("roundness"), sender);
	}

	void cElement::set_frame_thickness(float t, void* sender)
	{
		if (t == frame_thickness)
			return;
		frame_thickness = t;
		mark_dirty();
		data_changed(FLAME_CHASH("frame_thickness"), sender);
	}

	void cElement::set_color(const Vec4c& c, void* sender)
	{
		if (c == color)
			return;
		color = c;
		mark_dirty();
		data_changed(FLAME_CHASH("color"), sender);
	}

	void cElement::set_frame_color(const Vec4c& c, void* sender)
	{
		if (c == frame_color)
			return;
		frame_color = c;
		mark_dirty();
		data_changed(FLAME_CHASH("frame_color"), sender);
	}

	cElement* _linked_object;

	void cElement::set_linked_object(cElement* e)
	{
		_linked_object = e;
	}

	cElement* cElement::get_linked_object()
	{
		return _linked_object;
	}

	cElement* cElement::create()
	{
		return new cElementPrivate();
	}
}
