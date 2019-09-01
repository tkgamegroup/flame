#include <flame/graphics/canvas.h>
#include <flame/universe/components/element.h>

namespace flame
{
	struct cElementPrivate : cElement
	{

		cElementPrivate(graphics::Canvas* _canvas)
		{
			p_element = nullptr;

			x = 0.f;
			y = 0.f;
			scale = 1.f;
			width = 0.f;
			height = 0.f;

			inner_padding = Vec4f(0.f);

			alpha = 1.f;

			draw = true;
			background_round_radius = 0.f;
			background_round_flags = SideNW | SideNE | SideSE | SideSW;
			background_frame_thickness = 0.f;
			background_color = Vec4c(0);
			background_frame_color = Vec4c(255);
			background_shadow_thickness = 0.f;

			clip_children = false;

			canvas = _canvas;

			global_x = 0.f;
			global_y = 0.f;
			global_scale = 0.f;
			global_width = 0.f;
			global_height = 0.f;
		}

		void on_entity_added_to_parent()
		{
			auto e = entity->parent();
			if (e)
				p_element = (cElementPrivate*)(e->find_component(cH("Element")));
			else
				p_element = nullptr;
		}

		void start()
		{
			auto e = entity->parent();
			if (e)
				p_element = (cElementPrivate*)(e->find_component(cH("Element")));
			if (p_element)
				canvas = p_element->canvas;
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

			if (clip_children || !p_element)
			{
				auto cp = Vec2f(global_x, global_y) + Vec2f(inner_padding[0], inner_padding[1]) * global_scale;
				auto cs = Vec2f(global_width, global_height) - Vec2f(inner_padding[0] + inner_padding[2], inner_padding[1] + inner_padding[3]) * global_scale;
				scissor = Vec4f(cp, cp + cs);
			}
			else
			{
				scissor = p_element->scissor;
				canvas->set_scissor(scissor);
			}

			cliped = false;
			if (draw)
			{
				auto p = Vec2f(global_x, global_y);
				auto s = Vec2f(global_width, global_height);

				if (!p_element || rect_overlapping(p_element->scissor, Vec4f(p, p + s)))
				{
					auto rr = background_round_radius * global_scale;

					if (background_shadow_thickness > 0.f)
					{
						std::vector<Vec2f> points;
						path_rect(points, p - Vec2f(background_shadow_thickness * 0.5f), s + Vec2f(background_shadow_thickness), rr, (Side)background_round_flags);
						points.push_back(points[0]);
						canvas->stroke(points, Vec4c(0, 0, 0, 128), Vec4c(0), background_shadow_thickness);
					}
					if (alpha > 0.f)
					{
						std::vector<Vec2f> points;
						path_rect(points, p, s, rr, (Side)background_round_flags);
						if (background_color.w() > 0)
							canvas->fill(points, alpha_mul(background_color, alpha));
						if (background_frame_thickness > 0.f && background_frame_color.w() > 0)
						{
							points.push_back(points[0]);
							canvas->stroke(points, alpha_mul(background_frame_color, alpha), background_frame_thickness);
						}
					}
				}
				else
					cliped = true;
			}
		}
	};

	cElement::~cElement()
	{
	}

	void cElement::on_entity_added_to_parent()
	{
		((cElementPrivate*)this)->on_entity_added_to_parent();
	}

	void cElement::start()
	{
		((cElementPrivate*)this)->start();
	}

	void cElement::update()
	{
		((cElementPrivate*)this)->update();
	}

	cElement* cElement::create(graphics::Canvas* canvas)
	{
		return new cElementPrivate(canvas);
	}
}
