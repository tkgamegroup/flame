#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "element_private.h"
#include "bloom_private.h"

namespace flame
{
	void cBloomPrivate::draw0(graphics::Canvas* canvas)
	{
		canvas->add_bloom();
	}

	cBloom* cBloom::create()
	{
		return f_new<cBloomPrivate>();
	}
}
