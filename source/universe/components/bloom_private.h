#pragma once

#include "../entity_private.h"
#include <flame/universe/components/bloom.h>

namespace flame
{
	struct cElementPrivate;

	struct cBloomPrivate : cBloom
	{
		cElementPrivate* element = nullptr;

		void draw0(graphics::Canvas* canvas);
	};
}
