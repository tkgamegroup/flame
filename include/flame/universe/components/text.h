#pragma once

#include <flame/graphics/font.h>
#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct FontAtlas;
	}

	struct cElement;
	struct cAligner;

	struct cText : Component
	{
		cElement* element;
		cAligner* aligner;

		graphics::FontAtlas* font_atlas;
		uint font_size;
		Vec4c color;
		bool auto_size;

		cText() :
			Component("cText")
		{
		}

		FLAME_UNIVERSE_EXPORTS uint text_length() const;
		FLAME_UNIVERSE_EXPORTS const wchar_t* text() const;
		FLAME_UNIVERSE_EXPORTS void set_text(const wchar_t* text, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_font_size(uint s, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_color(const Vec4c& c, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_auto_size(bool v, void* sender = nullptr);

		FLAME_UNIVERSE_EXPORTS static cText* create(graphics::FontAtlas* font_atlas);
	};
}
