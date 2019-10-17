#pragma once

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
		Vec4c color;
		float sdf_scale;
		bool auto_width;
		bool auto_height;

		cText() :
			Component("Text")
		{
		}

		FLAME_UNIVERSE_EXPORTS const std::wstring& text() const;
		FLAME_UNIVERSE_EXPORTS void set_text(const std::wstring& text);

		FLAME_UNIVERSE_EXPORTS static cText* create(graphics::FontAtlas* font_atlas);
	};

	FLAME_UNIVERSE_EXPORTS Entity* create_standard_button(graphics::FontAtlas* font_atlas, float sdf_scale, const std::wstring& text);
	FLAME_UNIVERSE_EXPORTS Entity* wrap_standard_text(Entity* e, bool before, graphics::FontAtlas* font_atlas, float sdf_scale, const std::wstring& text);
}
