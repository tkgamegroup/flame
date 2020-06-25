#include <flame/graphics/canvas.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/image.h>

namespace flame
{
	struct cImagePrivate : cImage
	{
		void* draw_cmd;

		cImagePrivate()
		{
			element = nullptr;

			id = 0;
			uv0 = 0.f;
			uv1 = 1.f;
			color = 255;

			draw_cmd = nullptr;
		}

		~cImagePrivate()
		{
			if (!entity->dying_)
				element->cmds.remove(draw_cmd);
		}

		void on_event(EntityEvent e, void* t) override
		{
			if (e == EntityComponentAdded && t == this)
			{
				element = entity->get_component(cElement);
				assert(element);

				draw_cmd = element->cmds.add([](Capture& c, graphics::Canvas* canvas) {
					c.thiz<cImagePrivate>()->draw(canvas);
					return true;
				}, Capture().set_thiz(this));
			}
		}

		void draw(graphics::Canvas* canvas)
		{
			if (!element->clipped)
			{
				canvas->add_image(element->content_min(), element->content_size(), id, 
					uv0, uv1, color.copy().factor_w(element->alpha));
			}
		}
	};

	cImage* cImage::create()
	{
		return new cImagePrivate();
	}
}
