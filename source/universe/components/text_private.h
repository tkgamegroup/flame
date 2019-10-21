#pragma once

#include <flame/universe/components/text.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct cTextPrivate : cText
	{
		std::wstring text;

		cTextPrivate(graphics::FontAtlas* font_atlas);
		void draw(graphics::Canvas* canvas);
		void on_component_added(Component* c) override;
		Component* copy() override;
	};
}
