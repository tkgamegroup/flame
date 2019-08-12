#include <flame/graphics/canvas.h>
#include <flame/universe/components/element.h>

namespace flame
{
	struct cElementPrivate : cElement
	{
		cElementPrivate* p_element;

		cElementPrivate(Entity* e, graphics::Canvas* _canvas) :
			cElement(e),
			p_element(nullptr)
		{
			x = 0.f;
			y = 0.f;
			scale = 1.f;
			width = 0.f;
			height = 0.f;
			global_x = 0.f;
			global_y = 0.f;
			global_scale = 0.f;
			global_width = 0.f;
			global_height = 0.f;

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

			canvas = _canvas;
		}

		void on_add_to_parent()
		{
			auto e = entity->parent();
			if (e)
				p_element = (cElementPrivate*)(e->find_component(cH("Element")));
			if (!canvas)
			{
				canvas = p_element->canvas;
				assert(canvas);
			}
		}

		void update()
		{
			if (!p_element)
			{
				global_x = x;
				global_y = y;
				global_scale = scale;
			}
			else
			{
				global_x = p_element->global_x + p_element->global_scale * x;
				global_y = p_element->global_y + p_element->global_scale * y;
				global_scale = p_element->global_scale * scale;
			}
			global_width = width * global_scale;
			global_height = height * global_scale;

			if (canvas)
			{
				auto p = Vec2f(global_x, global_y) - (Vec2f(background_offset[0], background_offset[1])) * global_scale;
				auto s = Vec2f(global_width, global_height) + (Vec2f(background_offset[0] + background_offset[2], background_offset[1] + background_offset[3])) * global_scale;
				auto rr = background_round_radius * global_scale;

				if (background_shadow_thickness > 0.f)
				{
					std::vector<Vec2f> points;
					path_rect(points, p - Vec2f(background_shadow_thickness * 0.5f), s + Vec2f(background_shadow_thickness), rr, (Side)background_round_flags);
					points.push_back(points[0]);
					canvas->stroke(points, Vec4c(0, 0, 0, 128), Vec4c(0), background_shadow_thickness, true);
				}
				if (alpha > 0.f)
				{
					std::vector<Vec2f> points;
					path_rect(points, p, s, rr, (Side)background_round_flags);
					if (background_color.w() > 0)
						canvas->fill(points, alpha_mul(background_color, alpha));
					if (background_frame_thickness > 0.f && background_frame_color.w() > 0)
						canvas->fill(points, alpha_mul(background_frame_color, alpha));
				}
			}
		}
	};

	cElement::cElement(Entity* e) :
		Component("Element", e)
	{
	}

	cElement::~cElement()
	{
	}

	void cElement::on_add_to_parent()
	{
		((cElementPrivate*)this)->on_add_to_parent();
	}

	void cElement::update()
	{
		((cElementPrivate*)this)->update();
	}

	cElement* cElement::create(Entity* e, graphics::Canvas* canvas)
	{
		return new cElementPrivate(e, canvas);
	}
}
