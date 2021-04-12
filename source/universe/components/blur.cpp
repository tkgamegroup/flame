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
		if (entity)
			entity->component_data_changed(this, S<"blur_radius"_h>);
	}

	void cBlurPrivate::draw(sRenderer* renderer)
	{
		// TODO: fix below
		//if (blur_radius > 0)
		//	renderer->add_blur(element->aabb, blur_radius);
	}

	cBlur* cBlur::create(void* parms)
	{
		return f_new<cBlurPrivate>();
	}
}
