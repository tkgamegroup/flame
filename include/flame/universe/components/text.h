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
		float sdf_scale;
		bool auto_size;


		cText() :
			Component("Text")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cText() override;

		FLAME_UNIVERSE_EXPORTS virtual void on_added() override;

		FLAME_UNIVERSE_EXPORTS const std::wstring& text() const;
		FLAME_UNIVERSE_EXPORTS void set_text(const std::wstring& text);

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cText* create(graphics::FontAtlas* font_atlas);
	};
}
