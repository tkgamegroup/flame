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
		if (pos.x() == x)
			return;
		pos.x() = x;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"x"_h>);
	}

	void cElementPrivate::set_y(float y)
	{
		if (pos.y() == y)
			return;
		pos.y() = y;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"y"_h>);
	}

	void cElementPrivate::set_width(float w)
	{
		if (size.x() == w)
			return;
		size.x() = w;
		content_size.x() = size.x() - padding.xz().sum();
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"width"_h>);
	}

	void cElementPrivate::set_height(float h)
	{
		if (size.y() == h)
			return;
		size.y() = h;
		content_size.y() = size.y() - padding.yw().sum();
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"height"_h>);
	}

	void cElementPrivate::set_padding(const Vec4f& p)
	{
		if (padding == p)
			return;
		padding = p;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"padding"_h>);
	}

	//	void cElement::set_roundness(const Vec4f& r, void* sender)
	//	{
	//		if (r == roundness)
	//			return;
	//		roundness = r;
	//		mark_drawing_dirty();
	//		report_data_changed(FLAME_CHASH("roundness"), sender);
	//	}

	void cElementPrivate::set_pivotx(float p)
	{
		if (pivot.x() == p)
			return;
		pivot.x() = p;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"pivotx"_h>);
	}

	void cElementPrivate::set_pivoty(float p)
	{
		if (pivot.y() == p)
			return;
		pivot.y() = p;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"pivoty"_h>);
	}

	void cElementPrivate::set_scalex(float s)
	{
		if (scale.x() == s)
			return;
		scale.x() = s;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"scalex"_h>);
	}

	void cElementPrivate::set_scaley(float s)
	{
		if (scale.y() == s)
			return;
		scale.y() = s;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"scaley"_h>);
	}

	void cElementPrivate::set_rotation(float r)
	{
		if (rotation == r)
			return;
		rotation = r;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"rotation"_h>);
	}

	void cElementPrivate::set_skewx(float s)
	{
		if (skew.x() == s)
			return;
		skew.x() = s;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"skewx"_h>);
	}

	void cElementPrivate::set_skewy(float s)
	{
		if (skew.y() == s)
			return;
		skew.y() = s;
		mark_transform_dirty();
		Entity::report_data_changed(this, S<"skewy"_h>);
	}

	void cElementPrivate::update_transform()
	{
		if (transform_dirty)
		{
			transform_dirty = false;

			crooked = rotation == 0.f && skew == 0.f;
			auto base_axes = Mat2f(1.f);
			auto base_pos = Vec2f(0.f);
			auto pe = entity->get_parent_component_t<cElementPrivate>();
			if (pe)
			{
				pe->update_transform();
				crooked = crooked && pe->crooked;
				base_axes = pe->global_axes;
				base_pos = pe->global_points[0];
			}

			global_axes = base_axes;
			auto c = base_pos + global_axes * pos;
			global_axes[0] = make_rotation_matrix((rotation + skew.y()) * ANG_RAD) * global_axes[0];
			global_axes[1] = make_rotation_matrix((rotation + skew.x()) * ANG_RAD) * global_axes[1];
			global_axes = global_axes * Mat2f(scale);

			global_points[0] = c + global_axes * -pivot * size;
			global_points[1] = global_points[0] + global_axes[0] * size.x();
			global_points[2] = global_points[0] + global_axes * size;
			global_points[3] = global_points[0] + global_axes[1] * size.y();

			global_points[4] = global_points[0] + global_axes * padding.xy();
			global_points[5] = global_points[1] - global_axes[0] * padding.z() + global_axes[1] * padding.y();
			global_points[6] = global_points[2] - global_axes * padding.zw();
			global_points[7] = global_points[3] + global_axes[0] * padding.x() - global_axes[1] * padding.w();

			aabb = Vec4f(global_points[0], global_points[0]);
			for (auto i = 1; i < 4; i++)
			{
				aabb.x() = min(aabb.x(), global_points[i].x());
				aabb.z() = max(aabb.z(), global_points[i].x());
				aabb.y() = min(aabb.y(), global_points[i].y());
				aabb.w() = max(aabb.w(), global_points[i].y());
			}
		}
	}

	void cElementPrivate::set_fill_color(const Vec4c& c)
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

	void cElementPrivate::set_border_color(const Vec4c& c)
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

	bool cElementPrivate::contains(const Vec2f& p)
	{
		if (size == 0.f)
			return false;
		update_transform();
		if (crooked)
		{
			Vec2f ps[] = { global_points[0], global_points[1], global_points[2], global_points[3] };
			return convex_contains<float>(p, ps);
		}
		else
			return rect_contains(aabb, p);
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
							measurables.emplace_back((Component*)p, (void(*)(Component*, Vec2f&))addr);
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
			if (fill_color.a() > 0)
			{
				canvas->begin_path();
				canvas->move_to(global_points[0]);
				canvas->line_to(global_points[1]);
				canvas->line_to(global_points[2]);
				canvas->line_to(global_points[3]);
				canvas->fill(fill_color);
			}

			if (border > 0.f && border_color.a() > 0)
			{
				canvas->begin_path();
				canvas->move_to(global_points[4]);
				canvas->line_to(global_points[5]);
				canvas->line_to(global_points[6]);
				canvas->line_to(global_points[7]);
				canvas->line_to(global_points[4]);
				canvas->stroke(border_color, border);
			}
		}
	}

	cElement* cElement::create()
	{
		return f_new<cElementPrivate>();
	}
}
