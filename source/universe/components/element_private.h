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
		void on_entered_world() override;
		void on_left_world() override;
		void on_visibility_changed() override;
		void on_position_changed() override;
	};
}
