#pragma once

#include <flame/universe/components/text.h>

namespace flame
{
	struct cTextPrivate : cText
	{
		std::wstring text;

		cTextPrivate(graphics::FontAtlas* font_atlas);
		~cTextPrivate();
		void on_added();
		void on_other_added(Component* c);
		void update();
	};
}
