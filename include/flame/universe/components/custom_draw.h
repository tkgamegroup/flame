#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct cElement;

	struct cCustomDraw : Component
	{
		cElement* element;

		cCustomDraw() :
			Component("CustomDraw")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cCustomDraw* create();
	};
}
