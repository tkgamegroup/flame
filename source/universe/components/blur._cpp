#include "../entity_private.h"
#include "element_private.h"
#include "blur_private.h"

namespace flame
{
	void cBlurPrivate::set_blur_radius(uint r)
	{
		if (blur_radius == r)
			return;
		blur_radius = r;
		if (element)
			element->mark_drawing_dirty();
		data_changed(this, S<"blur_radius"_h>);
	}

	void cBlurPrivate::draw(sRenderer* s_renderer)
	{
		// TODO: fix below
		//if (blur_radius > 0)
		//	s_renderer->add_blur(element->aabb, blur_radius);
	}

	cBlur* cBlur::create(void* parms)
	{
		return new cBlurPrivate();
	}
}
