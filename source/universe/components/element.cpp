#include <flame/graphics/canvas.h>
#include <flame/universe/entity.h>
#include <flame/universe/components/ui.h>
#include <flame/universe/components/element.h>

namespace flame
{
	struct cElementPrivate : cElement
	{
		cElementPrivate* p_element;
		graphics::Canvas* canvas;

		cElementPrivate() :
			p_element(nullptr),
			canvas(nullptr)
		{
			x.v = 0.f;
			x.frame = 0;
			y.v = 0.f;
			y.frame = 0;
			scale.v = 1.f;
			scale.frame = 0;
			width.v = 0.f;
			width.frame = 0;
			height.v = 0.f;
			height.frame = 0;
			global_x.v = 0.f;
			global_x.frame = -1;
			global_y.v = 0.f;
			global_y.frame = -1;
			global_scale.v = 0.f;
			global_scale.frame = -1;
			global_width.v = 0.f;
			global_width.frame = -1;
			global_height.v = 0.f;
			global_height.frame = -1;

			inner_padding = Vec4f(0.f);
			layout_padding = 0.f;

			alpha = 1.f;

			background_offset = Vec4f(0.f);
			background_round_radius = 0.f;
			background_round_flags = 0;
			background_frame_thickness = 0.f;
			background_color = Vec4c(0);
			background_frame_color = Vec4c(255);
			background_shadow_thickness = 0.f;
		}

		void on_attach()
		{
			auto e = entity->parent();
			if (e)
			{
				p_element = (cElementPrivate*)(e->find_component(cH("Element")));
				if (p_element)
				{
					canvas = p_element->canvas;
					return;
				}
			}

			while (e)
			{
				auto c = (cUI*)(e->find_component(cH("UI")));
				if (c)
				{
					canvas = c->canvas();
					break;
				}
				e = e->parent();
			}
		}

		void update()
		{
			if (!p_element)
			{
				if (x.frame > global_x.frame)
					global_x = x;
				if (y.frame > global_y.frame)
					global_y = y;
				if (scale.frame > global_scale.frame)
					global_scale = scale;
			}
			else
			{
				if (x.frame > global_x.frame || p_element->global_x.frame > global_x.frame || p_element->global_scale.frame > global_x.frame)
				{
					global_x.v = p_element->global_x.v + p_element->global_scale.v * x.v;
					global_x.frame = maxN(p_element->global_x.frame, p_element->global_scale.frame, x.frame);
				}
				if (y.frame > global_y.frame || p_element->global_y.frame > global_y.frame || p_element->global_scale.frame > global_y.frame)
				{
					global_y.v = p_element->global_y.v + p_element->global_scale.v * y.v;
					global_y.frame = maxN(p_element->global_y.frame, p_element->global_scale.frame, y.frame);
				}
				if (scale.frame > global_scale.frame || p_element->global_scale.frame > global_scale.frame)
				{
					global_scale.v = p_element->global_scale.v * scale.v;
					global_scale.frame = max(p_element->global_scale.frame, scale.frame);
				}
			}
			if (width.frame > global_width.frame || global_scale.frame > global_width.frame)
			{
				global_width.v = width.v * global_scale.v;
				global_width.frame = max(width.frame, global_scale.frame);
			}
			if (height.frame > global_height.frame || global_scale.frame > global_height.frame)
			{
				global_height.v = height.v * global_scale.v;
				global_height.frame = max(height.frame, global_scale.frame);
			}

			if (canvas)
			{
				auto p = Vec2f(global_x.v, global_y.v) - (Vec2f(background_offset[0], background_offset[1])) * global_scale.v;
				auto s = Vec2f(global_width.v, global_height.v) + (Vec2f(background_offset[0] + background_offset[2], background_offset[1] + background_offset[3])) * global_scale.v;
				auto rr = background_round_radius * global_scale.v;

				if (background_shadow_thickness > 0.f)
				{
					canvas->add_rect_col2(p - Vec2f(background_shadow_thickness * 0.5f), s + Vec2f(background_shadow_thickness), Vec4c(0, 0, 0, 128), Vec4c(0),
						background_shadow_thickness, rr, (Side)background_round_flags);
				}
				if (alpha > 0.f)
				{
					if (background_color.w() > 0)
						canvas->add_rect_filled(p, s, Vec4c(Vec3c(background_color), background_color.w() * alpha), rr, (Side)background_round_flags);
					if (background_frame_thickness > 0.f && background_frame_color.w() > 0)
						canvas->add_rect(p, s, Vec4c(Vec3c(background_frame_color), background_frame_color.w() * alpha), background_frame_thickness, rr, (Side)background_round_flags);
				}
			}
		}
	};

	cElement::~cElement()
	{
	}

#define NAME "Element"
	const char* cElement::type_name() const
	{
		return NAME;
	}

	uint cElement::type_hash() const
	{
		return cH(NAME);
	}
#undef NAME

	void cElement::on_attach()
	{
		((cElementPrivate*)this)->on_attach();
	}

	void cElement::update()
	{
		((cElementPrivate*)this)->update();
	}

	graphics::Canvas* cElement::canvas() const
	{
		return ((cElementPrivate*)this)->canvas;
	}

	cElement* cElement::create()
	{
		return new cElementPrivate();
	}
}
