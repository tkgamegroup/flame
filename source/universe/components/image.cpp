#include <flame/universe/components/element.h>
#include "image_private.h"

#include "../renderpath/canvas_make_cmd/canvas.h"

namespace flame
{
	cImagePrivate::cImagePrivate()
	{
		element = nullptr;

		id = 0;
		uv0 = Vec2f(0.f);
		uv1 = Vec2f(1.f);
		color = Vec4c(255);
	}

	void cImagePrivate::on_component_added(Component* c)
	{
		if (c->name_hash == cH("Element"))
			element = (cElement*)c;
	}

	void cImagePrivate::draw(graphics::Canvas* canvas)
	{
		auto padding = element->inner_padding * element->global_scale;
		auto pos = element->global_pos + Vec2f(padding[0], padding[1]);
		auto size = element->global_size - Vec2f(padding[0] + padding[2], padding[1] + padding[3]);
		canvas->add_image(pos, size, id, uv0, uv1, alpha_mul(color, element->alpha));
	}

	cImage* cImage::create()
	{
		return new cImagePrivate();
	}
}
