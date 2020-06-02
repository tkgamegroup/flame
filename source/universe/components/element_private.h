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
		bool pending_sizing;

		cElementPrivate();
		~cElementPrivate();
		void calc_geometry();
		void draw(graphics::Canvas* canvas);
		void on_event(Entity::Event e, void* t) override;
	};
}
