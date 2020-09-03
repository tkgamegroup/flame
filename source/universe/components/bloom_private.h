#pragma once

#include <flame/universe/components/bloom.h>"

namespace flame
{
	struct cElementPrivate;

	struct cBloomPrivate : cBloom // R ~ on_*
	{
		cElementPrivate* element = nullptr; // R ref

		void draw_underlayer(graphics::Canvas* canvas); // R
	};
}