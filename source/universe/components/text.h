#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cText : Component
	{
		// Reflect
		std::wstring text;

		graphics::FontAtlas* font;

		// Reflect
		cvec4 color = cvec4(255);

		struct Create
		{
			virtual cTextPtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
