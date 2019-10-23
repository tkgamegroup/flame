#pragma once

#include <flame/universe/components/image.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct cImagePrivate : cImage
	{
		cImagePrivate();
		void on_component_added(Component* c) override;
		void draw(graphics::Canvas* canvas);
	};
}
