#include <flame/graphics/canvas.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/extra_element_drawing.h>

namespace flame
{
	struct cExtraElementDrawingPrivate : cExtraElementDrawing
	{
		void* draw_cmd;

		cExtraElementDrawingPrivate()
		{
			element = nullptr;

			draw_flags = (ExtraDrawFlag)0;
			color = 0;

			draw_cmd = nullptr;
		}

		~cExtraElementDrawingPrivate()
		{
			if (!entity->dying_)
				element->cmds.remove(draw_cmd);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cElement"))
			{
				element = (cElement*)c;
				draw_cmd = element->cmds.add([](Capture& c, graphics::Canvas* canvas) {
					c.thiz<cExtraElementDrawingPrivate>()->draw(canvas);
					return true;
				}, Capture().set_thiz(this));
			}
		}

		void draw(graphics::Canvas* canvas)
		{
			if (!element->clipped)
			{
				auto low = element->global_pos;
				auto high = low + element->global_size;
				if (draw_flags & ExtraDrawFilledCornerSE)
				{
					std::vector<Vec2f> points;
					points.push_back(Vec2f(high.x(), low.y()));
					points.push_back(Vec2f(low.x(), high.y()));
					points.push_back(Vec2f(high.x(), high.y()));
					canvas->fill(points.size(), points.data(), color.copy().factor_w(element->alpha));
				}
				if (draw_flags & ExtraDrawHorizontalLine)
				{
					auto y = (low.y() + high.y()) * 0.5f;
					std::vector<Vec2f> points;
					points.push_back(Vec2f(low.x(), y));
					points.push_back(Vec2f(high.x(), y));
					canvas->stroke(points.size(), points.data(), color.copy().factor_w(element->alpha), thickness * element->global_scale);
				}
			}
		}
	};

	cExtraElementDrawing* cExtraElementDrawing::create()
	{
		return new cExtraElementDrawingPrivate();
	}
}
