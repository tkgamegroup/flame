#pragma once

#include <flame/universe/components/element.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct cElementPrivate : cElement
	{
		cElementPrivate();
		void calc_geometry();
		void draw(graphics::Canvas* canvas);
		Component* copy() override;
	};
}
