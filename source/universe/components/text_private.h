#pragma once

#include <flame/universe/components/text.h>

namespace flame
{
	struct cTextPrivate : cText
	{
		std::wstring text;

		std::vector<std::unique_ptr<Closure<void(void* c, const wchar_t* text)>>> changed_listeners;

		cTextPrivate(graphics::FontAtlas* font_atlas);
		void on_changed();
		void update();
		Component* copy();
	};
}
