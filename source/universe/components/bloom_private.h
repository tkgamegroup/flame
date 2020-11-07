#pragma once

#include <flame/universe/components/bloom.h>"

namespace flame
{
	struct cElementPrivate;

	struct cBloomPrivate : cBloom // R ~ on_*
	{
		cElementPrivate* element = nullptr; // R ref

		void draw0(graphics::Canvas* canvas); // R
	};
}
