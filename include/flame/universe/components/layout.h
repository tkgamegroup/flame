#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cAligner;

	struct cLayout : Component
	{
		cElement* element;
		cAligner* aligner;

		LayoutType type;
		float item_padding;
		bool width_fit_children;
		bool height_fit_children;
		uint fence; // children that exceed fence will under free layout
		Vec2f scroll_offset_;
		uint column;

		Vec2f content_size;

		cLayout() :
			Component("Layout")
		{
		}

		FLAME_UNIVERSE_EXPORTS void set_x_scroll_offset(float x);
		FLAME_UNIVERSE_EXPORTS void set_y_scroll_offset(float y);

		FLAME_UNIVERSE_EXPORTS static cLayout* create(LayoutType type);
	};
}
