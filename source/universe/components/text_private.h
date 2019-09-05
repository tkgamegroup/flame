#pragma once

#include <flame/universe/components/text.h>

namespace flame
{
	struct cTextPrivate : cText
	{
		std::wstring text;

		cTextPrivate(graphics::FontAtlas* font_atlas);
		void start();
		void update();
		Component* copy();
	};
}
