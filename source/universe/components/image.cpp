#include <flame/universe/world.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/image.h>

#include "../renderpath/canvas/canvas.h"

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

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cElement"))
			{
				element = (cElement*)c;
				draw_cmd = element->cmds.add([](void* c, graphics::Canvas* canvas) {
					(*(cImagePrivate**)c)->draw(canvas);
					return true;
				}, Mail::from_p(this));
			}
		}

		void draw(graphics::Canvas* canvas)
		{
			if (!element->clipped)
			{
				canvas->add_image(element->content_min(), element->content_size(), id, 
					uv0, uv1, color.new_proply<3>(element->alpha));
			}
		}
	};

	cImage* cImage::create()
	{
		return new cImagePrivate();
	}
}
