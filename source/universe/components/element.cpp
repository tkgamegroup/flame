#include "../../graphics/canvas.h"
#include "../entity_private.h"
#include "element_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cElementPrivate::on_init()
	{
		drawers.add([this](graphics::CanvasPtr canvas) {
			if (!tilted)
			{
				if (background_col.a > 0)
					canvas->draw_rect_filled(global_pos0(), global_pos1(), background_col);
				if (frame_col.a > 0)
					canvas->draw_rect(global_pos0(), global_pos1(), frame_thickness, frame_col);
			}
			else
			{
				if (background_col.a > 0 || frame_col.a > 0)
				{
					vec2 pts[4];
					fill_pts(pts);
					if (background_col.a > 0)
					{
						canvas->path.assign_range(pts);
						canvas->fill(background_col);
					}
					if (frame_col.a > 0)
					{
						canvas->path.assign_range(pts);
						canvas->stroke(frame_thickness, frame_col, true);
					}
				}
			}
		});
	}

	void cElementPrivate::set_pos(const vec2& p)
	{
		if (pos == p)
			return;
		pos = p;
		mark_transform_dirty();
		data_changed("pos"_h);
	}

	void cElementPrivate::set_global_pos(const vec2& pos)
	{
		if (auto pelement = entity->get_parent_component<cElementT>(); pelement)
		{
			auto gscl = pelement->global_scl();
			set_pos((pos - pelement->global_pos()) / (gscl == vec2(0.f) ? 1.f : 0.f));
		}
		else
			set_pos(pos);
	}

	void cElementPrivate::set_ext(const vec2& e)
	{
		if (ext == e)
			return;
		ext = e;
		mark_transform_dirty();
		data_changed("ext"_h);
	}

	void cElementPrivate::set_global_ext(const vec2& ext)
	{
		if (auto pelement = entity->get_parent_component<cElementT>(); pelement)
		{
			auto gscl = pelement->global_scl();
			set_ext(ext / (gscl == vec2(0.f) ? 1.f : 0.f));
		}
		else
			set_ext(ext);
	}

	void cElementPrivate::set_ang(float a)
	{
		if (ang == a)
			return;
		ang = a;
		mark_transform_dirty();
		data_changed("ang"_h);
	}

	void cElementPrivate::set_scl(const vec2& s)
	{
		if (scl == s)
			return;
		scl = s;
		mark_transform_dirty();
		data_changed("scl"_h);
	}

	void cElementPrivate::set_background_col(const cvec4& col)
	{
		if (background_col == col)
			return;
		background_col = col;
		mark_drawing_dirty();
		data_changed("background_col"_h);
	}

	void cElementPrivate::set_frame_col(const cvec4& col)
	{
		if (frame_col == col)
			return;
		frame_col = col;
		mark_drawing_dirty();
		data_changed("frame_col"_h);
	}

	void cElementPrivate::set_frame_thickness(float thickness)
	{
		if (frame_thickness == thickness)
			return;
		frame_thickness = thickness;
		mark_drawing_dirty();
		data_changed("frame_thickness"_h);
	}

	void cElementPrivate::set_scissor(bool v)
	{
		if (scissor == v)
			return;
		scissor = v;
		mark_drawing_dirty();
		data_changed("scissor"_h);
	}

	void cElementPrivate::set_pivot(const vec2& _pivot)
	{
		if (pivot == _pivot)
			return;
		pivot = _pivot;
		mark_transform_dirty();
		data_changed("pivot"_h);
	}

	void cElementPrivate::set_horizontal_alignment(ElementAlignment alignment)
	{
		if (horizontal_alignment == alignment)
			return;
		horizontal_alignment = alignment;
		mark_transform_dirty();
		data_changed("horizontal_alignment"_h);
	}

	void cElementPrivate::set_vertical_alignment(ElementAlignment alignment)
	{
		if (vertical_alignment == alignment)
			return;
		vertical_alignment = alignment;
		mark_transform_dirty();
		data_changed("vertical_alignment"_h);
	}

	void cElementPrivate::set_margin(const vec4& _margin)
	{
		if (margin == _margin)
			return;
		margin = _margin;
		mark_transform_dirty();
		data_changed("margin"_h);
	}

	vec2 cElementPrivate::global_scl()
	{
		auto ret = scl;
		auto pelement = entity->get_parent_component<cElementT>();
		while (pelement)
		{
			ret *= pelement->scl;
			pelement = pelement->entity->get_parent_component<cElementT>();
		}
		return ret;
	}

	void cElementPrivate::mark_transform_dirty()
	{
		transform_dirty = true;
		mark_drawing_dirty();
	}

	void cElementPrivate::mark_drawing_dirty()
	{
		if (entity->depth != (ushort)-1)
			sRenderer::instance()->dirty = true;
	}

	bool cElementPrivate::update_transform()
	{
		if (!transform_dirty)
			return false;

		mat3 m;
		if (auto pelement = entity->get_parent_component<cElementT>(); pelement)
		{
			m = pelement->transform;
			tilted = pelement->tilted;
		}
		else
		{
			m = mat3(1.f);
			tilted = false;
		}
		m = translate(m, pos - pivot * ext * scl);
		m = rotate(m, radians(ang));
		m = scale(m, scl);
		transform = m;
		tilted |= ang != 0.f;

		data_changed("transform"_h);
		transform_dirty = false;

		return true;
	}

	void cElementPrivate::update_transform_from_root()
	{
		std::vector<cElementPtr> elements;
		auto e = this;
		while (e)
		{
			elements.push_back(e);
			e = e->entity->get_parent_component<cElementT>();
		}
		for (auto it = elements.rbegin(); it != elements.rend(); it++)
		{
			auto e = *it;
			e->transform_dirty = true;
			e->update_transform();
			e->mark_transform_dirty(); // remark dirty
		}
	}

	struct cElementCreate : cElement::Create
	{
		cElementPtr operator()(EntityPtr) override
		{
			return new cElementPrivate();
		}
	}cElement_create;
	cElement::Create& cElement::create = cElement_create;
}
