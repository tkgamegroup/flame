#include <flame/universe/components/element.h>
#include <flame/universe/components/image.h>

#include "../renderpath/canvas_make_cmd/canvas.h"

namespace flame
{
	struct cImagePrivate : cImage
	{
		cImagePrivate()
		{
			element = nullptr;

			id = 0;
			uv0 = Vec2f(0.f);
			uv1 = Vec2f(1.f);
			color = Vec4c(255);
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
		}

		void update()
		{
			if (!element->cliped)
			{
				auto padding = element->inner_padding * element->global_scale;
				auto pos = element->global_pos + Vec2f(padding[0], padding[1]);
				auto size = element->global_size - Vec2f(padding[0] + padding[2], padding[1] + padding[3]);
				element->canvas->add_image(pos, size, id, uv0, uv1, alpha_mul(color, element->alpha));
			}
		}
	};

	void cImage::start()
	{
		((cImagePrivate*)this)->start();
	}

	void cImage::update()
	{
		((cImagePrivate*)this)->update();
	}

	cImage* cImage::create()
	{
		return new cImagePrivate();
	}
}
