#pragma once

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
		Vec4c color;
		uint font_size_;
		float scale_;
		bool auto_width_;
		bool auto_height_;

		cText() :
			Component("cText")
		{
		}

		FLAME_UNIVERSE_EXPORTS const std::wstring& text() const;
		FLAME_UNIVERSE_EXPORTS void set_text(const std::wstring& text, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void insert_char(wchar_t ch, uint pos, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void erase_char(uint pos, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_font_size(uint s, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_auto_width(bool v, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_auto_height(bool v, void* sender = nullptr);

		FLAME_UNIVERSE_EXPORTS static cText* create(graphics::FontAtlas* font_atlas);
	};

	FLAME_UNIVERSE_EXPORTS Entity* create_standard_button(graphics::FontAtlas* font_atlas, float font_size_scale, const std::wstring& text);
	FLAME_UNIVERSE_EXPORTS Entity* wrap_standard_text(Entity* e, bool before, graphics::FontAtlas* font_atlas, float font_size_scale, const std::wstring& text);
}
