#include "../../foundation/typeinfo.h"
#include "../world_private.h"
#include "element_private.h"
#include "../systems/renderer_private.h"
#include "../systems/layout_private.h"

namespace flame
{
	void cElementPrivate::set_x(float x)
	{
		if (pos.x == x)
			return;
		pos.x = x;
		mark_transform_dirty();
		if (pelement)
			pelement->mark_layout_dirty();
		if (entity)
			entity->component_data_changed(this, S<"x"_h>);
	}

	void cElementPrivate::set_y(float y)
	{
		if (pos.y == y)
			return;
		pos.y = y;
		mark_transform_dirty();
		if (pelement)
			pelement->mark_layout_dirty();
		if (entity)
			entity->component_data_changed(this, S<"y"_h>);
	}

	void cElementPrivate::set_pos(const vec2& p)
	{
		if (pos == p)
			return;
		pos = p;
		mark_transform_dirty();
		if (pelement)
			pelement->mark_layout_dirty();
		if (entity)
			entity->component_data_changed(this, S<"pos"_h>);
	}

	void cElementPrivate::add_pos(const vec2& p)
	{
		set_pos(pos + p);
	}

	void cElementPrivate::set_width(float w)
	{
		if (size.x == w)
			return;
		size.x = w;
		content_size.x = size.x - padding[0] - padding[2];
		mark_transform_dirty();
		mark_layout_dirty();
		if (pelement)
			pelement->mark_layout_dirty();
		if (entity)
			entity->component_data_changed(this, S<"width"_h>);
	}

	void cElementPrivate::set_height(float h)
	{
		if (size.y == h)
			return;
		size.y = h;
		content_size.y = size.y - padding[1] - padding[3];
		mark_transform_dirty();
		mark_layout_dirty();
		if (pelement)
			pelement->mark_layout_dirty();
		if (entity)
			entity->component_data_changed(this, S<"height"_h>);
	}

	void cElementPrivate::set_size(const vec2& s)
	{
		if (size == s)
			return;
		size = s;
		content_size[0] = size.x - padding[0] - padding[2];
		content_size[1] = size.y - padding[1] - padding[3];
		mark_transform_dirty();
		mark_layout_dirty();
		if (pelement)
			pelement->mark_layout_dirty();
		if (entity)
			entity->component_data_changed(this, S<"size"_h>);
	}

	void cElementPrivate::add_size(const vec2& s)
	{
		set_size(size + s);
	}

	void cElementPrivate::set_padding(const vec4& p)
	{
		if (padding == p)
			return;
		padding = p;
		padding_size[0] = p[0] + p[2];
		padding_size[1] = p[1] + p[3];
		content_size = size - padding_size;
		mark_transform_dirty();
		mark_layout_dirty();
		if (entity)
			entity->component_data_changed(this, S<"padding"_h>);
	}

	void cElementPrivate::add_padding(const vec4& p)
	{
		set_padding(padding + p);
	}

	//	void cElement::set_roundness(const vec4& r, void* sender)
	//	{
	//		if (r == roundness)
	//			return;
	//		roundness = r;
	//		mark_drawing_dirty();
	//		report_data_changed(FLAME_CHASH("roundness"), sender);
	//	}

	void cElementPrivate::set_pivotx(float p)
	{
		if (pivot.x == p)
			return;
		pivot.x = p;
		mark_transform_dirty();
		if (entity)
			entity->component_data_changed(this, S<"pivotx"_h>);
	}

	void cElementPrivate::set_pivoty(float p)
	{
		if (pivot.y == p)
			return;
		pivot.y = p;
		mark_transform_dirty();
		if (entity)
			entity->component_data_changed(this, S<"pivoty"_h>);
	}

	void cElementPrivate::set_scalex(float s)
	{
		if (scl.x == s)
			return;
		scl.x = s;
		mark_transform_dirty();
		if (entity)
			entity->component_data_changed(this, S<"scalex"_h>);
	}

	void cElementPrivate::set_scaley(float s)
	{
		if (scl.y == s)
			return;
		scl.y = s;
		mark_transform_dirty();
		if (entity)
			entity->component_data_changed(this, S<"scaley"_h>);
	}

	void cElementPrivate::set_scale(const vec2& s)
	{
		if (scl == s)
			return;
		scl = s;
		mark_transform_dirty();
		if (entity)
			entity->component_data_changed(this, S<"scale"_h>);
	}

	void cElementPrivate::set_angle(float a)
	{
		if (angle == a)
			return;
		angle = a;
		mark_transform_dirty();
		if (entity)
			entity->component_data_changed(this, S<"angle"_h>);
	}

	void cElementPrivate::set_skewx(float s)
	{
		if (skew.x == s)
			return;
		skew.x = s;
		mark_transform_dirty();
		if (entity)
			entity->component_data_changed(this, S<"skewx"_h>);
	}

	void cElementPrivate::set_skewy(float s)
	{
		if (skew.y == s)
			return;
		skew.y = s;
		mark_transform_dirty();
		if (entity)
			entity->component_data_changed(this, S<"skewy"_h>);
	}

	void cElementPrivate::set_align_in_layout(bool v)
	{
		if (align_in_layout == v)
			return;
		align_in_layout = v;
		if (pelement)
			pelement->mark_layout_dirty();
		if (entity)
			entity->component_data_changed(this, S<"align_in_layout"_h>);
	}

	void cElementPrivate::set_align_absolute(bool a)
	{
		if (align_absolute == a)
			return;
		align_absolute = a;
		if (pelement)
			pelement->mark_layout_dirty();
		if (entity)
			entity->component_data_changed(this, S<"align_absolute"_h>);
	}

	void cElementPrivate::set_margin(const vec4& m)
	{
		if (margin == m)
			return;
		margin = m;
		margin_size[0] = m[0] + m[2];
		margin_size[1] = m[1] + m[3];
		if (pelement)
			pelement->mark_layout_dirty();
		if (entity)
			entity->component_data_changed(this, S<"margin"_h>);
	}

	void cElementPrivate::set_alignx(Align a)
	{
		if (alignx == a)
			return;
		alignx = a;
		if (pelement)
		{
			if (a != AlignNone)
				pelement->need_layout = true;
			pelement->mark_layout_dirty();
		}
		if (entity)
			entity->component_data_changed(this, S<"alignx"_h>);
	}

	void cElementPrivate::set_aligny(Align a)
	{
		if (aligny == a)
			return;
		aligny = a;
		if (pelement)
		{
			if (a != AlignNone)
				pelement->need_layout = true;
			pelement->mark_layout_dirty();
		}
		if (entity)
			entity->component_data_changed(this, S<"aligny"_h>);
	}

	void cElementPrivate::set_width_factor(float f)
	{
		if (width_factor == f)
			return;
		width_factor = f;
		if (pelement)
			pelement->mark_layout_dirty();
		if (entity)
			entity->component_data_changed(this, S<"width_factor"_h>);
	}

	void cElementPrivate::set_height_factor(float f)
	{
		if (height_factor == f)
			return;
		height_factor = f;
		if (pelement)
			pelement->mark_layout_dirty();
		if (entity)
			entity->component_data_changed(this, S<"height_factor"_h>);
	}

	void cElementPrivate::set_layout_type(LayoutType t)
	{
		need_layout = true;
		if (layout_type == t)
			return;
		layout_type = t;
		mark_layout_dirty();
		if (entity)
			entity->component_data_changed(this, S<"layout_type"_h>);
	}

	void cElementPrivate::set_layout_gap(float g)
	{
		if (layout_gap == g)
			return;
		layout_gap = g;
		mark_layout_dirty();
		if (entity)
			entity->component_data_changed(this, S<"layout_gap"_h>);
	}

	//	void cElementPrivate::set_column(uint c)
	//	{
	//		if (column == c)
	//			return;
	//		column = c;
	//		auto thiz = (cElementPrivate*)this;
	//		if (thiz->management)
	//			thiz->management->add_to_update_list(thiz);
	//	}

	void cElementPrivate::set_auto_width(bool a)
	{
		if (auto_width == a)
			return;
		auto_width = a;
		mark_size_dirty();
		mark_layout_dirty();
		if (entity)
			entity->component_data_changed(this, S<"auto_width"_h>);
	}

	void cElementPrivate::set_auto_height(bool a)
	{
		if (auto_height == a)
			return;
		auto_height = a;
		mark_size_dirty();
		mark_layout_dirty();
		if (entity)
			entity->component_data_changed(this, S<"auto_height"_h>);
	}

	void cElementPrivate::set_scrollx(float s)
	{
		if (scroll.x == s)
			return;
		scroll.x = s;
		mark_layout_dirty();
		if (entity)
			entity->component_data_changed(this, S<"scrollx"_h>);
	}

	void cElementPrivate::set_scrolly(float s)
	{
		if (scroll.y == s)
			return;
		scroll.y = s;
		mark_layout_dirty();
		if (entity)
			entity->component_data_changed(this, S<"scrolly"_h>);
	}

	void cElementPrivate::set_fill_color(const cvec4& c)
	{
		if (fill_color == c)
			return;
		fill_color = c;
		mark_drawing_dirty();
		if (entity)
			entity->component_data_changed(this, S<"fill_color"_h>);
	}

	void cElementPrivate::set_border(float b)
	{
		if (border == b)
			return;
		border = b;
		mark_drawing_dirty();
		if (entity)
			entity->component_data_changed(this, S<"border"_h>);
	}

	void cElementPrivate::set_border_color(const cvec4& c)
	{
		if (border_color == c)
			return;
		border_color = c;
		mark_drawing_dirty();
		if (entity)
			entity->component_data_changed(this, S<"border_color"_h>);
	}

	void cElementPrivate::set_clipping(bool c)
	{
		if (clipping == c)
			return;
		clipping = c;
		mark_drawing_dirty();
		if (entity)
			entity->component_data_changed(this, S<"clipping"_h>);
	}

	void* cElementPrivate::add_drawer(uint (*drawer)(Capture&, uint, sRendererPtr), const Capture& capture)
	{
		if (!drawer)
		{
			auto slot = (uint)&capture;
			drawer = [](Capture& c, uint, sRendererPtr render) {
				auto scr_ins = script::Instance::get_default();
				scr_ins->get_global("callbacks");
				scr_ins->get_member(nullptr, c.data<uint>());
				scr_ins->get_member("f");
				scr_ins->push_object();
				scr_ins->set_object_type("flame::sRenderer", render);
				scr_ins->call(1);
				auto ret = scr_ins->to_int(-1);
				scr_ins->pop(3);
				return (uint)ret;
			};
			auto c = new Closure(drawer, Capture().set_data(&slot));
			drawers.emplace_back(c);
			return c;
		}
		auto c = new Closure(drawer, capture);
		drawers.emplace_back(c);
		return c;
	}

	void cElementPrivate::remove_drawer(void* drawer)
	{
		std::erase_if(drawers, [&](const auto& i) {
			return i == (decltype(i))drawer;
		});
	}

	void* cElementPrivate::add_measurer(bool(*measurer)(Capture&, vec2*), const Capture& capture)
	{
		auto c = new Closure(measurer, capture);
		measurers.emplace_back(c);
		return c;
	}

	void cElementPrivate::remove_measurer(void* measurer)
	{
		std::erase_if(measurers, [&](const auto& i) {
			return i == (decltype(i))measurer;
		});
	}

	void cElementPrivate::update_transform()
	{
		if (transform_dirty)
		{
			transform_dirty = false;

			crooked = !(angle == 0.f && skew.x == 0.f && skew.y == 0.f);
			if (pelement)
			{
				pelement->update_transform();
				crooked = crooked && pelement->crooked;
				transform = pelement->transform;
			}
			else
				transform = mat3(1.f);

			transform = translate(transform, pos);
			if (crooked)
			{
				mat3 axes3 = mat3(1.f);
				axes3 = rotate(axes3, radians(angle));
				axes3 = shearX(axes3, skew.y);
				axes3 = shearY(axes3, skew.x);
				transform = transform * axes3;
			}
			transform = scale(transform, scl);
			axes = mat2(transform);
			axes_inv = inverse(axes);

			auto a = -pivot * size;
			auto b = a + size;
			vec2 c, d;
			if (crooked)
			{
				a = vec2(transform[2]) + axes * a;
				b = vec2(transform[2]) + axes * b;
				c = a + axes * padding.xy();
				d = b - axes * padding.zw();
			}
			else
			{
				a += vec2(transform[2]);
				b += vec2(transform[2]);
				c = a + padding.xy();
				d = b - padding.zw();
			}
			points[0] = vec2(a.x, a.y);
			points[1] = vec2(b.x, a.y);
			points[2] = vec2(b.x, b.y);
			points[3] = vec2(a.x, b.y);
			points[4] = vec2(c.x, c.y);
			points[5] = vec2(d.x, c.y);
			points[6] = vec2(d.x, d.y);
			points[7] = vec2(c.x, d.y);

			aabb.reset();
			for (auto i = 0; i < 4; i++)
				aabb.expand(points[i]);
		}
	}

	void cElementPrivate::mark_transform_dirty()
	{
		if (!transform_dirty)
		{
			transform_dirty = true;
			for (auto& c : entity->children)
			{
				auto e = c->get_component_i<cElementPrivate>(0);
				if (e)
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

	void cElementPrivate::mark_size_dirty()
	{
		if (pending_sizing || measurers.empty() || !(auto_width || auto_height) || !layout_system)
			return;

		auto it = layout_system->sizing_list.begin();
		for (; it != layout_system->sizing_list.end(); it++)
		{
			if ((*it)->entity->depth < entity->depth)
				break;
		}
		layout_system->sizing_list.emplace(it, this);
		pending_sizing = true;
	}

	void cElementPrivate::mark_layout_dirty()
	{
		if (pending_layout || !need_layout || !layout_system)
			return;

		auto it = layout_system->layout_list.begin();
		for (; it != layout_system->layout_list.end(); it++)
		{
			if (entity->depth < (*it)->entity->depth)
				break;
		}
		layout_system->layout_list.emplace(it, this);
		pending_layout = true;
	}

	void cElementPrivate::remove_from_sizing_list()
	{
		if (!pending_sizing)
			return;
		std::erase_if(layout_system->sizing_list, [&](const auto& i) {
			return i == this;
		});
		pending_sizing = false;
	}

	void cElementPrivate::remove_from_layout_list()
	{
		if (!pending_layout)
			return;
		std::erase_if(layout_system->layout_list, [&](const auto& i) {
			return i == this;
		});
		pending_layout = false;
	}

	void cElementPrivate::on_self_added()
	{
		pelement = entity->get_parent_component_t<cElementPrivate>();
		if (pelement)
			mark_transform_dirty();
	}

	void cElementPrivate::on_self_removed()
	{
		pelement = nullptr;
	}

	void cElementPrivate::on_child_added(EntityPtr e)
	{
		auto element = e->get_component_i<cElementPrivate>(0);
		if (element)
		{
			if (element->alignx != AlignNone || element->aligny != AlignNone)
				need_layout = true;
			mark_layout_dirty();
		}
	}

	void cElementPrivate::on_child_removed(EntityPtr e)
	{
		mark_layout_dirty();
	}

	void cElementPrivate::on_entered_world()
	{
		auto world = entity->world;
		if (!world->first_element)
			world->first_element = entity;
		renderer = world->get_system_t<sRendererPrivate>();
		fassert(renderer);
		layout_system = world->get_system_t<sLayoutPrivate>();
		fassert(layout_system);
		mark_transform_dirty();
		mark_size_dirty();
		mark_layout_dirty();
	}

	void cElementPrivate::on_left_world()
	{
		auto world = entity->world;
		if (world->first_element == entity)
			world->first_element = nullptr;
		mark_drawing_dirty();
		remove_from_sizing_list();
		remove_from_layout_list();
		renderer = nullptr;
		layout_system = nullptr;
	}

	void cElementPrivate::on_visibility_changed(bool v)
	{
		if (v)
		{
			mark_size_dirty();
			mark_layout_dirty();
		}
		else
		{
			remove_from_sizing_list();
			remove_from_layout_list();
		}
		if (pelement)
			pelement->mark_layout_dirty();
	}

	void cElementPrivate::on_reposition(uint from, uint to)
	{
		mark_drawing_dirty();
		if (pelement)
			pelement->mark_layout_dirty();
	}

	bool cElementPrivate::contains(const vec2& p)
	{
		if (size.x == 0.f || size.y == 0.f)
			return false;
		update_transform();
		if (crooked)
		{
			auto pp = axes_inv * (p - points[0]);
			return pp.x >= 0.f && pp.x <= size.x && pp.y >= 0.f && pp.y <= size.y;
		}
		else
			return aabb.contains(p);
	}

	bool cElementPrivate::on_save_attribute(uint h)
	{
		switch (h)
		{
		case S<"x"_h>:
		case S<"y"_h>:
		case S<"width"_h>:
		case S<"height"_h>:
		case S<"scalex"_h>:
		case S<"scaley"_h>:
			return false;
		case S<"size"_h>:
			if ((alignx == AlignMinMax && aligny == AlignMinMax) ||
				(auto_width && auto_height))
				return false;
			break;
		}
		return true;
	}

	void cElementPrivate::draw(uint layer, sRenderer* renderer)
	{
		if (alpha > 0.f)
		{
			if (fill_color.a > 0)
				renderer->fill_rect(layer, this, vec2(0.f), size, fill_color);
			if (border > 0.f && border_color.a > 0)
				renderer->stroke_rect(layer, this, vec2(0.f), size, border, border_color);
		}
	}

	cElement* cElement::create(void* parms)
	{
		return f_new<cElementPrivate>();
	}
}
