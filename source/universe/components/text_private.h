#pragma once

#include <flame/universe/components/text.h>

namespace flame
{
	struct cTextPrivate : cText
	{
		std::wstring text;

		cTextPrivate(Entity* e, graphics::FontAtlas* font_atlas);
		~cTextPrivate();
		void update();
	};
}
