#include "../../foundation/typeinfo.h"
#include "../world_private.h"
#include "element_private.h"
//#include "../systems/renderer_private.h"
//#include "../systems/scene_private.h"

namespace flame
{
	void cElementPrivate::set_pos(const vec2& p)
	{
		if (pos == p)
			return;
		pos = p;
		mark_transform_dirty();
		if (pelement)
			pelement->mark_layout_dirty();
		data_changed(S<"pos"_h>);
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
		data_changed(S<"size"_h>);
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
		data_changed(S<"padding"_h>);
	}

	//	void cElement::set_roundness(const vec4& r, void* sender)
	//	{
	//		if (r == roundness)
	//			return;
	//		roundness = r;
	//		mark_drawing_dirty();
	//		report_data_changed(FLAME_CHASH("roundness"), sender);
	//	}

	void cElementPrivate::set_pivot(const vec2& p)
	{
		if (pivot == p)
			return;
		pivot = p;
		mark_transform_dirty();
		data_changed(S<"pivot"_h>);
	}

	void cElementPrivate::set_scale(const vec2& s)
	{
		if (scl == s)
			return;
		scl = s;
		mark_transform_dirty();
		data_changed(S<"scale"_h>);
	}

	void cElementPrivate::set_angle(float a)
	{
		if (angle == a)
			return;
		angle = a;
		mark_transform_dirty();
		data_changed(S<"angle"_h>);
	}

	void cElementPrivate::set_skew(const vec2& s)
	{
		if (skew == s)
			return;
		skew = s;
		mark_transform_dirty();
		data_changed(S<"skew"_h>);
	}

	void cElementPrivate::set_align_in_layout(bool v)
	{
		if (align_in_layout == v)
			return;
		align_in_layout = v;
		if (pelement)
			pelement->mark_layout_dirty();
		data_changed(S<"align_in_layout"_h>);
	}

	void cElementPrivate::set_align_absolute(bool a)
	{
		if (align_absolute == a)
			return;
		align_absolute = a;
		if (pelement)
			pelement->mark_layout_dirty();
		data_changed(S<"align_absolute"_h>);
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
		data_changed(S<"margin"_h>);
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
		data_changed(S<"alignx"_h>);
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
		data_changed(S<"aligny"_h>);
	}

	void cElementPrivate::set_width_factor(float f)
	{
		if (width_factor == f)
			return;
		width_factor = f;
		if (pelement)
			pelement->mark_layout_dirty();
		data_changed(S<"width_factor"_h>);
	}

	void cElementPrivate::set_height_factor(float f)
	{
		if (height_factor == f)
			return;
		height_factor = f;
		if (pelement)
			pelement->mark_layout_dirty();
		data_changed(S<"height_factor"_h>);
	}

	void cElementPrivate::set_layout_type(LayoutType t)
	{
		need_layout = true;
		if (layout_type == t)
			return;
		layout_type = t;
		mark_layout_dirty();
		data_changed(S<"layout_type"_h>);
	}

	void cElementPrivate::set_layout_gap(float g)
	{
		if (layout_gap == g)
			return;
		layout_gap = g;
		mark_layout_dirty();
		data_changed(S<"layout_gap"_h>);
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
		data_changed(S<"auto_width"_h>);
	}

	void cElementPrivate::set_auto_height(bool a)
	{
		if (auto_height == a)
			return;
		auto_height = a;
		mark_size_dirty();
		mark_layout_dirty();
		data_changed(S<"auto_height"_h>);
	}

	void cElementPrivate::set_scroll(const vec2& s)
	{
		if (scroll == s)
			return;
		scroll = s;
		mark_layout_dirty();
		data_changed(S<"scrollx"_h>);
	}

	void cElementPrivate::set_fill_color(const cvec4& c)
	{
		if (fill_color == c)
			return;
		fill_color = c;
		mark_drawing_dirty();
		data_changed(S<"fill_color"_h>);
	}

	void cElementPrivate::set_border(float b)
	{
		if (border == b)
			return;
		border = b;
		mark_drawing_dirty();
		data_changed(S<"border"_h>);
	}

	void cElementPrivate::set_border_color(const cvec4& c)
	{
		if (border_color == c)
			return;
		border_color = c;
		mark_drawing_dirty();
		data_changed(S<"border_color"_h>);
	}

	void cElementPrivate::set_enable_clipping(bool c)
	{
		if (enable_clipping == c)
			return;
		enable_clipping = c;
		mark_drawing_dirty();
		data_changed(S<"enable_clipping"_h>);
	}

	void cElementPrivate::update_transform()
	{
		if (transform_dirty)
		{
			transform_dirty = false;

			crooked = !(angle == 0.f && skew.x == 0.f && skew.y == 0.f && scl.x == 1.f && scl.y == 1.f);
			if (pelement)
			{
				pelement->update_transform();
				crooked = crooked || pelement->crooked;
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

			bounds.reset();
			for (auto i = 0; i < 4; i++)
				bounds.expand(points[i]);
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
		if (s_renderer)
			s_renderer->dirty = true;
	}

	void cElementPrivate::mark_size_dirty()
	{
		if (pending_sizing || measurers.empty() || !(auto_width || auto_height) || !s_scene)
			return;

		s_scene->add_to_sizing(this);
		pending_sizing = true;
	}

	void cElementPrivate::mark_layout_dirty()
	{
		if (pending_layout || !need_layout || !s_scene)
			return;

		s_scene->add_to_layout(this);
		pending_layout = true;
	}

	void cElementPrivate::remove_from_sizing_list()
	{
		if (!pending_sizing)
			return;
		s_scene->remove_from_sizing(this);
		pending_sizing = false;
	}

	void cElementPrivate::remove_from_layout_list()
	{
		if (!pending_layout)
			return;
		s_scene->remove_from_layout(this);
		pending_layout = false;
	}

	void cElementPrivate::on_component_added(Component* c)
	{
		auto drawer = dynamic_cast<ElementDrawer*>(c);
		if (drawer)
			add_drawer(drawer);
		auto measurer = dynamic_cast<ElementMeasurer*>(c);
		if (measurer)
			add_measurer(measurer);
	}

	void cElementPrivate::on_component_removed(Component* c)
	{
		auto drawer = dynamic_cast<ElementDrawer*>(c);
		if (drawer)
			remove_drawer(drawer);
		auto measurer = dynamic_cast<ElementMeasurer*>(c);
		if (measurer)
			remove_measurer(measurer);
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

		s_scene = world->get_system_t<sScenePrivate>();
		assert(s_scene);
		s_renderer = world->get_system_t<sRendererPrivate>();
		assert(s_renderer);

		pelement = entity->get_parent_component_t<cElementPrivate>();

		mark_transform_dirty();
		mark_size_dirty();
		mark_layout_dirty();
	}

	void cElementPrivate::on_left_world()
	{
		auto world = entity->world;
		if (world->first_element == entity)
			world->first_element = nullptr;

		remove_from_sizing_list();
		remove_from_layout_list();
		mark_drawing_dirty();

		s_scene = nullptr;
		s_renderer = nullptr;

		pelement = nullptr;
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
			return bounds.contains(p);
	}

	bool cElementPrivate::draw(uint layer)
	{
		auto transparent = true;
		if (alpha > 0.f)
		{
			if (fill_color.a > 0)
			{
				transparent = false;
				s_renderer->fill(layer, 4, points, fill_color);
			}
			if (border > 0.f && border_color.a > 0)
			{
				transparent = false;
				s_renderer->stroke(layer, 4, points, border, border_color, true);
			}
		}
		return transparent;
	}

	cElement* cElement::create()
	{
		return new cElementPrivate();
	}
}
