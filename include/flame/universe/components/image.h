#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;

	struct cImage : Component
	{
		cElement* element;

		uint id;
		Vec2f uv0;
		Vec2f uv1;
		Vec4c color;

		cImage() :
			Component("cImage")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cImage* create();
	};
}
