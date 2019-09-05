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

		LayoutType$ type;
		float item_padding;
		bool width_fit_children;
		bool height_fit_children;
		Vec2f scroll_offset;

		Vec2f content_size;

		cLayout() :
			Component("Layout")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;
		FLAME_UNIVERSE_EXPORTS virtual Component* copy() override;

		FLAME_UNIVERSE_EXPORTS static cLayout* create();
	};
}
