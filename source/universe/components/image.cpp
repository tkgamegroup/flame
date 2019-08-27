#include <flame/graphics/canvas.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/image.h>

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
			stretch = false;
			border = Vec4f(0.f);
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
				auto pos = Vec2f(element->global_x, element->global_y) + Vec2f(padding[0], padding[1]);
				auto size = Vec2f(element->global_width - padding[0] - padding[2], element->global_height - padding[1] - padding[3]);
				if (!stretch)
					element->canvas->add_image(pos, size, id, uv0, uv1);
				else
					element->canvas->add_image_stretch(pos, size, id, border);
			}
		}
	};

	cImage::~cImage()
	{
	}

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
