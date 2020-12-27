#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "element_private.h"
#include "post_effect_private.h"

namespace flame
{
	void cPostEffectPrivate::set_blur_radius(uint r)
	{
		if (blur_radius == r)
			return;
		blur_radius = r;
		if (element)
			element->mark_drawing_dirty();
		if (entity)
			entity->data_changed(this, S<"blur_radius"_h>);
	}

	void cPostEffectPrivate::set_enable_bloom(bool v)
	{
		if (enable_bloom == v)
			return;
		enable_bloom = v;
		if (element)
			element->mark_drawing_dirty();
		if (entity)
			entity->data_changed(this, S<"enable_bloom"_h>);
	}

	void cPostEffectPrivate::draw(graphics::Canvas* canvas)
	{
		if (blur_radius > 0)
			canvas->add_blur(element->aabb, blur_radius);
		if (enable_bloom)
			canvas->add_bloom();
	}

	cPostEffect* cPostEffect::create()
	{
		return f_new<cPostEffectPrivate>();
	}
}
