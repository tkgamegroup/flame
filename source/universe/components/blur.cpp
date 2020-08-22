#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "blur_private.h"

namespace flame
{
	void cBlurPrivate::set_radius(uint r)
	{
		if (radius == r)
			return;
		radius = r;
		if (element)
			element->mark_drawing_dirty();
		Entity::report_data_changed(this, S<ch("radius")>::v);
	}

	void cBlurPrivate::on_gain_element()
	{
		element->before_drawers.push_back(this);
		element->mark_drawing_dirty();
	}

	void cBlurPrivate::on_lost_element()
	{
		std::erase_if(element->before_drawers, [&](const auto& i) {
			return i == this;
		});
		element->mark_drawing_dirty();
	}

	void cBlurPrivate::draw(graphics::Canvas* canvas)
	{
		canvas->add_blur(Vec4f(element->points[0], element->points[2]), radius);
	}

	cBlur* cBlur::create()
	{
		return f_new<cBlurPrivate>();
	}
}
