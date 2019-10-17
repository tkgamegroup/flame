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
		Vec2f scroll_offset;
		uint column;

		Vec2f content_size;

		cLayout() :
			Component("Layout")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual void on_component_added(Component* c) override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;
		FLAME_UNIVERSE_EXPORTS virtual Component* copy() override;

		FLAME_UNIVERSE_EXPORTS static cLayout* create(LayoutType type);
	};
}
