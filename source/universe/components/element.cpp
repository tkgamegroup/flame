#include <flame/foundation/typeinfo.h>
#include <flame/graphics/canvas.h>
#include "../world_private.h"
#include "element_private.h"
#include "../systems/renderer_private.h"
#include "../systems/layout_system_private.h"

namespace flame
{
	void cElementPrivate::set_x(float x)
	{
		if (pos.x == x)
			return;
		pos.x = x;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"x"_h>);
	}

	void cElementPrivate::set_y(float y)
	{
		if (pos.y == y)
			return;
		pos.y = y;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"y"_h>);
	}

	void cElementPrivate::set_width(float w)
	{
		if (size.x == w)
			return;
		size.x = w;
		content_size.x = size.x - padding[0] - padding[2];
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"width"_h>);
	}

	void cElementPrivate::set_height(float h)
	{
		if (size.y == h)
			return;
		size.y = h;
		content_size.y = size.y - padding[1] - padding[3];
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"height"_h>);
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
		Entity::report_data_changed(this, S<"padding"_h>);
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
		Entity::report_data_changed(this, S<"pivotx"_h>);
	}

	void cElementPrivate::set_pivoty(float p)
	{
		if (pivot.y == p)
			return;
		pivot.y = p;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"pivoty"_h>);
	}

	void cElementPrivate::set_scalex(float s)
	{
		if (scl.x == s)
			return;
		scl.x = s;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"scalex"_h>);
	}

	void cElementPrivate::set_scaley(float s)
	{
		if (scl.y == s)
			return;
		scl.y = s;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"scaley"_h>);
	}

	void cElementPrivate::set_angle(float a)
	{
		if (angle == a)
			return;
		angle = a;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"angle"_h>);
	}

	void cElementPrivate::set_skewx(float s)
	{
		if (skew.x == s)
			return;
		skew.x = s;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"skewx"_h>);
	}

	void cElementPrivate::set_skewy(float s)
	{
		if (skew.y == s)
			return;
		skew.y = s;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"skewy"_h>);
	}

	void cElementPrivate::update_transform()
	{
		if (transform_dirty)
		{
			transform_dirty = false;

			crooked = !(angle == 0.f && skew.x == 0.f && skew.y == 0.f);
			auto m = mat3(1.f);
			auto pe = entity->get_parent_component_t<cElementPrivate>();
			if (pe)
			{
				pe->update_transform();
				crooked = crooked && pe->crooked;
				m = pe->transform;
			}

			m = translate(m, pos);
			if (crooked)
			{
				mat3 axes3 = mat3(1.f);
				axes3 = rotate(axes3, radians(angle));
				axes3 = shearX(axes3, skew.y);
				axes3 = shearY(axes3, skew.x);
				m = m * axes3;
				axes = mat2(axes3);
			}
			else
				axes = mat2(1.f);
			m = scale(m, scl);

			auto a = -pivot * size;
			auto b = a + size;
			auto c = a;
			auto d = b;
			if (crooked)
			{
				a = vec2(m[2]) + axes * a;
				b = vec2(m[2]) + axes * b;
				c += axes * padding.xy();
				d -= axes * padding.zw();
			}
			else
			{
				a += vec2(m[2]);
				b += vec2(m[2]);
				c += padding.xy();
				d -= padding.zw();
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

	void cElementPrivate::set_fill_color(const cvec4& c)
	{
		if (fill_color == c)
			return;
		fill_color = c;
		mark_drawing_dirty();
		Entity::report_data_changed(this, S<"fill_color"_h>);
	}

	void cElementPrivate::set_border(float b)
	{
		if (border == b)
			return;
		border = b;
		mark_drawing_dirty();
		Entity::report_data_changed(this, S<"border"_h>);
	}

	void cElementPrivate::set_border_color(const cvec4& c)
	{
		if (border_color == c)
			return;
		border_color = c;
		mark_drawing_dirty();
		Entity::report_data_changed(this, S<"border_color"_h>);
	}

	void cElementPrivate::set_clipping(bool c)
	{
		if (clipping == c)
			return;
		clipping = c;
		mark_drawing_dirty();
		Entity::report_data_changed(this, S<"clipping"_h>);
	}

	void cElementPrivate::mark_transform_dirty()
	{
		if (!transform_dirty)
		{
			transform_dirty = true;
			for (auto& c : entity->children)
			{
				auto e = c->get_component_t<cElementPrivate>();
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
		if (layout_system)
			layout_system->add_to_sizing_list(this);
	}

	void cElementPrivate::on_gain_renderer()
	{
		mark_transform_dirty();
	}

	void cElementPrivate::on_lost_renderer()
	{
		mark_transform_dirty();
	}

	void cElementPrivate::on_gain_layout_system()
	{
		mark_size_dirty();
	}

	bool cElementPrivate::contains(const vec2& p)
	{
		if (size.x == 0.f || size.y == 0.f)
			return false;
		update_transform();
		if (crooked)
		{
			vec2 ps[] = { points[0], points[1], points[2], points[3] };
			return convex_contains(p, ps);
		}
		else
			return aabb.contains(p);
	}

	void cElementPrivate::on_local_message(Message msg, void* p)
	{
		switch (msg)
		{
		case MessageComponentAdded:
		{
			auto udt = find_underlay_udt(((Component*)p)->type_name);
			if (udt)
			{
				{
					auto f = udt->find_function("draw0");
					if (f && f->check(TypeInfo::get(TypeData, "void"), TypeInfo::get(TypePointer, "flame::graphics::Canvas"), nullptr))
					{
						auto addr = f->get_address();
						if (addr)
						{
							drawers[0].emplace_back((Component*)p, (void(*)(Component*, graphics::Canvas*))addr);
							mark_drawing_dirty();
						}
					}
				}
				{
					auto f = udt->find_function("draw");
					if (f && f->check(TypeInfo::get(TypeData, "void"), TypeInfo::get(TypePointer, "flame::graphics::Canvas"), nullptr))
					{
						auto addr = f->get_address();
						if (addr)
						{
							drawers[1].emplace_back((Component*)p, (void(*)(Component*, graphics::Canvas*))addr);
							mark_drawing_dirty();
						}
					}
				}
				{
					auto f = udt->find_function("measure");
					if (f && f->check(TypeInfo::get(TypeData, "void"), TypeInfo::get(TypePointer, "flame::Vec<2,float>"), nullptr))
					{
						auto addr = f->get_address();
						if (addr)
						{
							measurables.emplace_back((Component*)p, (void(*)(Component*, vec2&))addr);
							mark_size_dirty();
						}
					}
				}
			}
		}
			break;
		case MessageComponentRemoved:
		{
			if (std::erase_if(measurables, [&](const auto& i) {
				return i.first == (Component*)p;
			}))
				mark_size_dirty();
			auto n = std::erase_if(drawers[0], [&](const auto& i) {
				return i.first == (Component*)p;
			}) + std::erase_if(drawers[1], [&](const auto& i) {
				return i.first == (Component*)p;
			});
			if (n)
				mark_drawing_dirty();
		}
			break;
		case MessageVisibilityChanged:
			mark_size_dirty();
		case MessageElementTransformDirty:
			mark_transform_dirty();
			break;
		case MessageElementSizeDirty:
			mark_size_dirty();
			break;
		case MessagePositionChanged:
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
			if (fill_color.a > 0)
			{
				canvas->begin_path();
				canvas->move_to(points[0]);
				canvas->line_to(points[1]);
				canvas->line_to(points[2]);
				canvas->line_to(points[3]);
				canvas->fill(fill_color);
			}

			if (border > 0.f && border_color.a > 0)
			{
				canvas->begin_path();
				canvas->move_to(points[0]);
				canvas->line_to(points[1]);
				canvas->line_to(points[2]);
				canvas->line_to(points[3]);
				canvas->close_path();
				canvas->stroke(border_color, border);
			}
		}
	}

	cElement* cElement::create()
	{
		return f_new<cElementPrivate>();
	}
}
