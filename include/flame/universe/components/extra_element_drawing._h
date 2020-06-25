#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;

	struct cExtraElementDrawing : Component
	{
		cElement* element;

		ExtraDrawFlag draw_flags;
		float thickness;
		Vec4c color;

		cExtraElementDrawing() :
			Component("cExtraElementDrawing")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cExtraElementDrawing* create();
	};
}
