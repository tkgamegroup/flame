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
		~cElementPrivate();
		void calc_geometry();
		void draw(graphics::Canvas* canvas);
		void on_entered_world() override;
		void on_left_world() override;
		Component* copy() override;
	};
}
