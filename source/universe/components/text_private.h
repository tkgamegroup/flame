#pragma once

#include <flame/universe/components/text.h>

namespace flame
{
	namespace graphics
	{
		struct Glyph;
		struct Canvas;
	}

	struct cTextPrivate : cText
	{
		std::wstring text;
		uint draw_font_size;
		std::vector<graphics::Glyph*> glyphs;

		cTextPrivate(graphics::FontAtlas* font_atlas);
		void update_glyphs();
		void draw(graphics::Canvas* canvas);
		void on_component_added(Component* c) override;
		Component* copy() override;
	};
}
