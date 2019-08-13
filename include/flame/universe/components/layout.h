#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;

	struct cLayout : Component
	{
		cElement* element;

		LayoutType$ type;
		float item_padding;
		bool clip;
		bool fit_children;

		float scroll_offset;

		FLAME_UNIVERSE_EXPORTS cLayout(Entity* e);
		FLAME_UNIVERSE_EXPORTS virtual ~cLayout() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cLayout* create(Entity* e);
	};
}
