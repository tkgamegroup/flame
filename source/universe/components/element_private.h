#pragma once

#include <flame/universe/components/element.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct sElementRenderer;

	struct cElementPrivate : cElement
	{
		sElementRenderer* renderer;

		//cElementPrivate();
		//~cElementPrivate();

		//void calc_geometry();
		//void draw(graphics::Canvas* canvas);
		//void on_event(EntityEvent e, void* t) override;
	};
}
