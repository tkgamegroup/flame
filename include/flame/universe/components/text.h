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

	struct cText : Component
	{
		cElement* element;

		graphics::FontAtlas* font_atlas;
		uint font_size_;
		float scale_;
		Vec4c color_;
		bool auto_width_;
		bool auto_height_;

		cText() :
			Component("cText")
		{
		}

		FLAME_UNIVERSE_EXPORTS uint text_length() const;
		FLAME_UNIVERSE_EXPORTS const wchar_t* text() const;
		FLAME_UNIVERSE_EXPORTS void set_text(const wchar_t* text, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_font_size(uint s, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_scale(float s, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_color(const Vec4c& c, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_auto_width(bool v, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_auto_height(bool v, void* sender = nullptr);

		FLAME_UNIVERSE_EXPORTS static cText* create(graphics::FontAtlas* font_atlas);
	};
}
