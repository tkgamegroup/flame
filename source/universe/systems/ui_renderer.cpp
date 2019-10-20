#include "../entity_private.h"
#include <flame/universe/systems/ui_renderer.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/aligner.h>

#include "../../../flame/renderpath/canvas_make_cmd/canvas.h"

namespace flame
{
	struct sUIRendererPrivate : sUIRenderer
	{
		graphics::Canvas* canvas;

		sUIRendererPrivate(graphics::Canvas* canvas) :
			canvas(canvas)
		{
		}

		void draw_element_and_its_children(cElement* c_e)
		{
			auto e = (EntityPrivate*)c_e->entity;

			c_e->cliped = !rect_overlapping(canvas->scissor(), Vec4f(c_e->global_pos, c_e->global_pos + c_e->global_size));
			if (!c_e->cliped)
			{
				auto r = c_e->roundness * c_e->global_scale;
				auto st = c_e->shadow_thickness * c_e->global_scale;

				if (st > 0.f)
				{
					std::vector<Vec2f> points;
					path_rect(points, c_e->global_pos - Vec2f(st * 0.5f), c_e->global_size + Vec2f(st), r);
					points.push_back(points[0]);
					canvas->stroke(points, Vec4c(0, 0, 0, 128), Vec4c(0), st);
				}
				if (c_e->alpha > 0.f)
				{
					std::vector<Vec2f> points;
					path_rect(points, c_e->global_pos, c_e->global_size, r);
					if (c_e->color.w() > 0)
						canvas->fill(points, alpha_mul(c_e->color, c_e->alpha));
					auto ft = c_e->frame_thickness * c_e->global_scale;
					if (ft > 0.f && c_e->frame_color.w() > 0)
					{
						points.push_back(points[0]);
						canvas->stroke(points, alpha_mul(c_e->frame_color, c_e->alpha), ft);
					}
				}

				auto c_t = e->get_component(Text);
				if (c_t)
				{
					auto rect = canvas->add_text(c_t->font_atlas, c_e->global_pos +
						Vec2f(c_e->inner_padding[0], c_e->inner_padding[1]) * c_e->global_scale,
						alpha_mul(c_t->color, c_e->alpha), c_t->text().c_str(), c_t->sdf_scale * c_e->global_scale);
					if (c_t->auto_width)
					{
						auto w = rect.x() * c_t->sdf_scale + c_e->inner_padding_horizontal();
						if (c_t->aligner && c_t->aligner->width_policy == SizeGreedy)
						{
							c_t->aligner->min_size.x() = w;
							c_e->size.x() = max(c_e->size.x(), w);
						}
						else
							c_e->size.x() = w;
					}
					if (c_t->auto_height)
					{
						auto h = rect.y() * c_t->sdf_scale + c_e->inner_padding_vertical();
						if (c_t->aligner && c_t->aligner->width_policy == SizeGreedy)
						{
							c_t->aligner->min_size.y() = h;
							c_e->size.y() = max(c_e->size.y(), h);
						}
						else
							c_e->size.y() = h;
					}
				}
			}

			for (auto& c : e->children)
				do_render(c.get());
		}

		void do_render(Entity* e)
		{
			if (!e->global_visibility_)
				return;

			auto element = e->get_component(Element);
			if (!element)
				return;

			auto p = e->parent();
			if (!p)
			{
				element->global_pos = element->pos;
				element->global_scale = element->scale;
			}
			else
			{
				auto p_element = p->get_component(Element);
				element->global_pos = p_element->global_pos + p_element->global_scale * element->pos;
				element->global_scale = p_element->global_scale * element->scale;
			}
			element->global_size = element->size * element->global_scale;

			if (element->clip_children)
			{
				auto last_scissor = canvas->scissor();
				auto scissor = Vec4f(element->global_pos, element->global_pos + element->global_size);
				scissor += Vec4f(element->inner_padding[0], element->inner_padding[1], element->inner_padding_horizontal(), element->inner_padding_vertical()) * element->global_scale;
				canvas->set_scissor(scissor);
				draw_element_and_its_children(element);
				canvas->set_scissor(last_scissor);
			}
			else
				draw_element_and_its_children(element);
		}

		void update(Entity* root) override
		{
			do_render(root);
		}
	};

	sUIRenderer* sUIRenderer::create(graphics::Canvas* canvas)
	{
		return new sUIRendererPrivate(canvas);
	}
}
